# How to fetch freertos svn

## 1. prepare freertos local folder

```bash
$ mkdir freertos_svn
$ cd freertos_svn
$ git svn clone --prefix=svn/ -sr 2320:HEAD svn://svn.code.sf.net/p/freertos/code
$ git gc
```

`-s` means using standard svn conventions.

## 2. merge changes from svn

### 2.1 Create local branch

```bash
$ git branch -m svn
$ git branch svn
```

### 2.2 Fetch updates

```bash
$ git checkout svn
$ git svn fetch
$ git checkout master
$ git svn rebase
```
