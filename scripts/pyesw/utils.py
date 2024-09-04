# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module acts as a supporting module for all the other modules. It
contains APIs for small use cases to avoid rewriting of code for those
generic requirements. It doesnt have any main() function and running
this module independently is not intended.
"""

import os
import sys
import re
import glob
import fileinput
import shutil
import yaml
import time
import logging
import json
import subprocess
from pathlib import Path
from distutils.dir_util import copy_tree
from typing import Any, List, Optional, Dict, Union
from collections.abc import MutableMapping
from functools import wraps

def get_logger(name):
    return logging.getLogger(name)

def log_time(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        start_time = time.time()
        result = func(*args, **kwargs)
        end_time = time.time()
        logger = get_logger(__name__)
        duration = end_time - start_time
        if 'log_message' in kwargs:
            log_message = kwargs['log_message']
            if log_message != None:
                logger.info(f"{log_message} took {duration:.2f} seconds")
        else:
            logger.info(f"Function {func.__name__} took {duration:.2f} seconds")
        return result
    return wrapper

def delete_keys_from_dict(dictionary: dict, keys: str) -> dict:
    """
    Delete keys from the dict. It can detect the key even inside the
    hierarchical dict.

    Args:
        | dictionary : The dictionary to be processed
        | keys : The key name that needs to be searched and popped out
    Returns:
        modified_dict : The new dict modified after popping the key
    """
    modified_dict = {}
    for key, value in dictionary.items():
        if key not in keys:
            if isinstance(value, MutableMapping):
                modified_dict[key] = delete_keys_from_dict(value, keys)
            else:
                modified_dict[key] = value
    return modified_dict


def is_file(filepath: str, silent_discard: bool = True) -> bool:
    """Return True if the file exists Else returns False and raises Not Found Error Message.

    Args:
        filepath: File Path.
    Raises:
        FileNotFoundError: Raises exception if file not found.
    Returns:
        bool: True, if file is found Or False, if file is not found.
    """

    if os.path.isfile(filepath):
        return True
    elif not silent_discard:
        err_msg = f"No such file exists: {filepath}"
        raise FileNotFoundError(err_msg) from None
    else:
        return False

def is_dir(dirpath: str, silent_discard: bool = True) -> bool:
    """Checks if directory exists.

    Args:
        dirpath: Directory Path.
    Raises:
        ValueError (Exception): Raises exception if directory not found.
    Returns:
        bool: True, if directory is found Or False, if directory is not found.
    """

    if os.path.isdir(dirpath):
        return True
    elif not silent_discard:
        err_msg = f"No such directory exists: {dirpath}"
        raise ValueError(err_msg) from None
    else:
        return False


def remove(path: str, silent_discard: bool = True, pattern: bool=False) -> None:
    """Removes any file or folder recursively, if it exists else reports error message based on user demand.

    Args:
        path: Directory or file path or a pattern.
        silent_discard: True if exceptions are to be ignored.
        pattern: True when the passed path is a pattern.
    Raises:
        Exception: Raises exception if any remove action fails.

    """
    try:
        if pattern:
            for entry in glob.glob(path):
                if is_file(entry):
                    os.remove(entry)
                elif is_dir(entry):
                    shutil.rmtree(entry)
        elif is_file(path):
            os.remove(path)
        elif is_dir(path):
            shutil.rmtree(path)
    except Exception as e:
        assert silent_discard, e


def mkdir(folderpath: str, silent_discard: bool = True) -> None:
    """Create the folder structure, raises Error Message on demand.

    Args:
        folderpath: Path of the folder structure.
    """
    is_successful = False
    try:
        os.makedirs(folderpath)
        is_successful = True
    except:
        pass

    if not silent_discard:
        if is_successful:
            print("%s Directory created " % folderpath)
        else:
            print("%s Unable to create directory " % folderpath)
            sys.exit(1)

def copy_file(src: str, dest: str, follow_symlinks: bool = False, silent_discard: bool = False) -> None:
    """
    copies the file from source to destination.

    Args:
        | src: source file path
        | dest: destination file path
        | follow_symlinks: maintain the symlink while copying
        | silent_discard: Dont raise exception if the source file doesnt exist
    """
    is_file(src, silent_discard)
    try:
        if is_dir(dest):
            dest = os.path.join(dest, os.path.basename(src))
        shutil.copyfile(src, dest, follow_symlinks=follow_symlinks)
        change_permission(dest, 0o644)
    except Exception as e:
        assert silent_discard, e

def change_permission(directory_path, permissions):
    """
    Recursively sets permissions for files under the specified path.
    :param directory_path: The root directory path.
    :param permissions: The desired permissions (e.g., 0o755 for read, write, and execute).
    """
    for root, dirs, files in os.walk(directory_path):
        for dir_name in dirs:
            dir_path = os.path.join(root, dir_name)
            if os.path.isdir(dir_path):
                change_permission(dir_path, permissions)

        # Set permissions for files
        for file_name in files:
            file_path = os.path.join(root, file_name)
            os.chmod(file_path, permissions)


def copy_directory(src: str, dst: str, symlinks: bool = False, ignore=None) -> None:
    """
    copies the directory from source to destination.

    Args:
        | src: source directory path
        | dest: destination directory path
        | symlinks: maintain the symlink while copying
        | ignore: provide list to ignore files/sub-dirs if any
    """
    if not is_dir(dst):
        mkdir(dst)
    for item in os.listdir(src):
        s = os.path.join(src, item)
        d = os.path.join(dst, item)
        if is_dir(s):
            if is_dir(d):
                copy_tree(s, d, symlinks)
                change_permission(d, 0o644)
            else:
                shutil.copytree(s, d, symlinks, ignore)
                change_permission(d, 0o644)
        else:
            copy_file(s, d)
            change_permission(d, 0o644)


def reset(path: str) -> None:
    """
    Delete the passed path and then recreate it.

    Args:
        path: Path that needs to be reset
    """
    remove(path)
    mkdir(path)

def validate_if_not_exist(config_file: str, dir_type: str, dir_path: str) -> None:
    """
    Raise valid assertion when a file doesnt exist

    Args:
        | config_file : File Path that needs to be checked
        | dir_type, dir_name: Being used for raising a meaningful assertion.
    """
    assert is_file(config_file), f"{dir_type.title()} at {dir_path} doesnt exist. Create the {dir_type} first."

def validate_if_exist(config_file: str, dir_type: str, dir_path: str) -> None:
    """
    Raise valid assertion when a file already exists

    Args:
        | config_file : File Path that needs to be checked
        | dir_type, dir_path: Being used for raising a meaningful assertion.
    """
    assert not is_file(config_file), f"{dir_type.title()} at {dir_path} already exists. Cannot create a new {dir_type} with same name."

def fetch_yaml_data(config_file: str, dir_type: str) -> Optional[dict]:
    """
    Reads the data from a yaml configuration file, raises assertion if file
    doesn't exist.

    Args:
        | config_file: The yaml configuration file path
        | dir_type: Being used for raising a meaningful assertion.
    Returns:
        data: The read data from yaml file
    """
    assert is_file(config_file), f"Could not find valid {dir_type} at {get_dir_path(config_file)}"
    data = load_yaml(config_file)
    return data

def load_yaml(filepath: str) -> Optional[dict]:
    """Read yaml file data and returns data in a dict format.

    Args:
        filepath: Path of the yaml file.
    Returns:
        dict: Return Python dict if the file reading is successful.
    """

    if is_file(filepath):
        try:
            with open(filepath) as f:
                data = yaml.safe_load(f)
            return data
        except:
            print("%s file reading failed" % filepath)
            return None
    else:
        return None

def write_yaml(filepath: str, data):
    """
    Write the data into a yaml file format

    Args:
        | filepath: the yaml file path
        | data: the data
    """
    with open(filepath, 'w') as outfile:
        yaml.dump(data, outfile, default_flow_style=False, sort_keys=False)

def update_yaml(filepath: str, dir_type: str, key: str, data: Optional[dict], action: str="add"):
    """
    Update the already created yaml file. Supports the add and remove option
    to add any new data or remove the existing data in the yaml. Raises assertion
    if the yaml doesnt exist.

    Args:
        | filepath: the yaml path
        | dir_type: to raise a meaningful assertion
        | key: Key that needs to be manipulated.
        | data: The new data that needs to be updated in yaml in 'add' use case.
        | action: 'add'/'remove'

    """
    new_data = fetch_yaml_data(filepath, dir_type)
    if action == "add":
        try:
            if isinstance(data, dict):
                new_data[key] = {**new_data[key] , **data}
            elif isinstance(data, list):
                new_data[key] += data
            else:
                new_data[key] = data
        except KeyError:
            new_data[key] = data
    elif action == "remove":
        new_data = delete_keys_from_dict(new_data, key)

    write_yaml(filepath, new_data)

def add_newline(File: str, newline: str) -> None:
    """
    Add a new line at the end of the file.

    Args:
        | File: file path which needs to be modified.
        | newline: new line that needs to be added.
    """
    with open(File, "a") as fd:
        fd.write(newline + "\n")

def remove_line(File: str, match_string: str) -> None:
    """
    Remove the lines that match the passed pattern

    Args:
        | File: file path which needs to be modified.
        | match_string: the string that needs to be searched in the line.
    """
    with open(File, "r+") as f:
        new_f = f.readlines()
        f.seek(0)
        for line in new_f:
            if match_string not in line:
                f.write(line)
        f.truncate()

def replace_line(File: str, search_string: str, add_line: str) -> None:
    """
    Replace an existing line that matches the passed string with a new line

    Args:
        | File: file path which needs to be modified.
        | search_string: the string that needs to be searched in the line.
        | add_line: New line that needs to be put in.
    """
    if is_file(File) == True:
        add_line = add_line + "\n"
        for line in fileinput.input(File, inplace=True):
            if search_string in line:
                line = add_line
            sys.stdout.write(line)
    else:
        err_msg = f"No such file exists: {File}"
        raise FileNotFoundError(err_msg)

def replace_string(File, search_string: str, replace_string: str) -> None:
    """
    Replace an existing string that matches the passed string.

    Args:
        | File: file path which needs to be modified.
        | search_string: the string that needs to be searched.
        | replace_string: the string that needs to be replaced.
    """
    if is_file(File):
        with open(File, "r+") as fd:
            filedata = fd.read()
            filedata = re.sub(search_string, replace_string, filedata)
            fd.seek(0)
            fd.write(filedata)
            fd.truncate()
    else:
        err_msg = f"No such file exists: {File}"
        raise FileNotFoundError(err_msg)

@log_time
def runcmd(cmd, cwd=None, logfile=None, log_message=None) -> bool:
    """
    Run the shell commands.

    Args:
        | cmd: shell command that needs to be called
        | logfile: file to save the command output if required
    """
    ret = True
    if logfile is None:
        try:
            subprocess.check_call(cmd, cwd=cwd, shell=True)
        except subprocess.CalledProcessError as exc:
            ret = False
            sys.exit(1)
    else:
        try:
            subprocess.check_call(cmd, cwd=cwd, shell=True, stdout=logfile, stderr=logfile)
        except subprocess.CalledProcessError:
            ret = False
    return ret

def get_base_name(fpath):
    """
    This api takes rel path or full path and returns base name

    Args:
        fpath: Path to get the base name from.
    Returns:
        string: Base name of the path
    """
    return os.path.basename(fpath.rstrip(os.path.sep))


def get_dir_path(fpath):
    """
    This api takes file path and returns it's directory path

    Args:
        fpath: Path to get the directory path from.
    Returns:
        string: Full Directory path of the passed path
    """
    return os.path.dirname(fpath.rstrip(os.path.sep))

def get_abs_path(fpath):
    """
    This api takes file path and returns it's absolute path

    Args:
        fpath: Path to get the absolute path from.
    Returns:
        string: Absolute location of the passed path
    """
    return os.path.abspath(fpath)


def get_original_path(fpath):
    """
    This api takes file path and returns it's original path. It is equivalent
    to readlink

    Args:
        fpath: Path to get the original path from.
    Returns:
        string: original location of the passed path (after resolving softlink if any)
    """
    return os.path.realpath(fpath)


def get_rel_path(curr_path, ref_path):
    """
    This api takes current path and returns it's relative path with reference to the
    ref_path.

    Args:
        | curr_path: Path for which the relative path is needed.
        | ref_path: Reference path to get the relative path from.
    Returns:
        string: the relative path between ref_path and curr_path
    """
    return os.path.relpath(curr_path, ref_path)


def find_file(search_file: str, search_path: str):
    """
    This api find the file in sub-directories and returns absolute path of
    file, if file exists

    Args:
        | search_file: The regex pattern to be searched in file names
        | search_path: The directory that needs to be searched
    Returns:
        string: Path of the first file that matches the pattern
    """
    for File in Path(search_path).glob(f"**/{search_file}"):
        return File

def find_files(search_pattern, search_path):
    """
    This api find the files matching regex directories and returns absolute
    path of files, if file exists

    Args:
        | search_pattern: The regex pattern to be searched in file names
        | search_path: The directory that needs to be searched
    Returns:
        string: All the file paths that matches the pattern in the searched path.

    """

    return glob.glob(f"{search_path}/{search_pattern}")

def check_if_line_in_file(
    file_name: str, line_to_search: str) -> bool:
    """Check if line exist in file or not"""
    with open(file_name, "r") as read_obj:
        file_data = read_obj.readlines()
        for data in file_data:
            if line_to_search in data:
                return True
    return False

def write_into_file(out_file, content):
    with open(out_file, 'w') as f:
        f.write(content)

def get_cmake_generator():
    if os.name == "nt":
        cmake_generator = "Ninja"
    else:
        cmake_generator = "Unix Makefiles"
    return cmake_generator

def discard_dump():
    if os.name == "nt":
        return "NUL"
    else:
        return "/dev/null"

def touch(filepath: str):
    Path(filepath).touch()

def get_domain_name(proc_name: str, yaml_file: str):
    schema = fetch_yaml_data(yaml_file, "domains")["domains"]
    for subsystem in schema:
        if schema[subsystem].get("domains", {}):
            for dom in schema[subsystem]["domains"]:
                domain_name = schema[subsystem]["domains"][dom]["cpus"][0]["cluster_cpu"]
                if domain_name == proc_name:
                    return dom
    return None

def get_high_precedence_path(repo_paths_list, file_type, *argv):
    path = ""
    for entries in repo_paths_list:
        path = os.path.join(
            entries, *argv
        )
        if is_file(path) or is_dir(path):
            break
    if not path:
        print(f"[ERROR]: Couldnt find the {file_type} in any of esw paths passed")
        sys.exit(1)
    return path

def load_json(filepath: str, silent_discard: bool = True) -> Optional[dict]:
    """Read json file data and returns data in a dict format.

    Args:
        filepath: Path of the json file.
        silent_discard: Ignore assertion if required.
    Returns:
        dict: Return Python dict if the file reading is successful.
    """

    try:
        with open(filepath) as f:
            data = json.load(f)
        return data
    except:
        assert silent_discard, f"{filepath} reading failed"
        return None
