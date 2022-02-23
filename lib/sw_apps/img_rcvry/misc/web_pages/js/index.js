function onPageLoad() {
	document.getElementById("upld_prgrs").style.visibility = "hidden";
	document.getElementById("upld_status").style.visibility = "hidden";

	var http = new XMLHttpRequest();
	http.open("GET", "sys_info", true);
	http.send();

	http.onload = function() {
		var obj = JSON.parse(this.responseText);

		var table = document.getElementById("sysboardtbl");
		table.rows[0].cells[1].innerHTML = obj.SysBoardInfo.BoardName;
		table.rows[1].cells[1].innerHTML = obj.SysBoardInfo.RevisionNo;
		table.rows[2].cells[1].innerHTML = obj.SysBoardInfo.SerialNo;
		table.rows[3].cells[1].innerHTML = obj.SysBoardInfo.PartNo;
		table.rows[4].cells[1].innerHTML = obj.SysBoardInfo.UUID;

		table = document.getElementById("cctbl");
		table.rows[0].cells[1].innerHTML = obj.CcInfo.BoardName;
		table.rows[1].cells[1].innerHTML = obj.CcInfo.RevisionNo;
		table.rows[2].cells[1].innerHTML = obj.CcInfo.SerialNo;
		table.rows[3].cells[1].innerHTML = obj.CcInfo.PartNo;
		table.rows[4].cells[1].innerHTML = obj.CcInfo.UUID;

		if (obj.SysBoardInfo.BoardName.startsWith("SMK-")) {
			document.getElementById("recWICLabel").style.display = "none";
			document.getElementById("recWICimg").disabled = true;
			document.getElementById("recWICimg").style.display = "none";
		}

		updateBootImgStatus();
	}
	document.getElementById("upld_btn").addEventListener("CrcDone", onCrcComplete);
	document.getElementById("upld_btn").addEventListener("FlashEraseDone", initiateImgUpload);

}

function updateBootImgStatus() {
	var http = new XMLHttpRequest();
	http.open("GET", "boot_img_status", true);
	http.send();

	http.onload = function() {
		var obj = JSON.parse(this.responseText);
		var SysImgInfoTbl = document.getElementById("sysimginfotbl");

		if (obj.ImgABootable == true) {
			document.getElementById("imgAb").checked = true;
			SysImgInfoTbl.rows[0].cells[2].innerHTML = "Bootable";
		}
		else {
			document.getElementById("imgAnb").checked = true;
			SysImgInfoTbl.rows[0].cells[2].innerHTML = "Non Bootable";
		}

		if (obj.ImgBBootable == true) {
			document.getElementById("imgBb").checked = true;
			SysImgInfoTbl.rows[1].cells[1].innerHTML = "Bootable";
		}
		else {
			document.getElementById("imgBnb").checked = true;
			SysImgInfoTbl.rows[1].cells[1].innerHTML = "Non Bootable";
		}

		if (obj.ReqBootImg == "ImageA") {
			document.getElementById("reqAimg").checked = true;
			SysImgInfoTbl.rows[2].cells[1].innerHTML = "Image A";
		}
		else {
			document.getElementById("reqBimg").checked = true;
			SysImgInfoTbl.rows[2].cells[1].innerHTML = "Image B";
		}

		if (obj.LastBootImg == "ImageA") {
			document.getElementById("recBimg").checked = true;
			SysImgInfoTbl.rows[3].cells[1].innerHTML = "Image A";
		}
		else {
			document.getElementById("recAimg").checked = true;
			SysImgInfoTbl.rows[3].cells[1].innerHTML = "Image B";
		}
	}
}

function onCfg() {
	var obj = { ImgABootable: false, ImgBBootable: false, ReqBootImg:"" };
	var http = new XMLHttpRequest();

	http.open("POST", "cfg_boot_img", true);
	http.setRequestHeader("Content-type","application/json;charset=UTF-8");
	if(document.getElementById("imgAb").checked)
		obj.ImgABootable = true;
	if(document.getElementById("imgBb").checked)
		obj.ImgBBootable = true;

	if(document.getElementById("reqAimg").checked)
		obj.ReqBootImg = "ImageA";
	else if(document.getElementById("reqBimg").checked)
		obj.ReqBootImg = "ImageB";
	var params = JSON.stringify(obj);
	http.send(params);

	http.onload = function() {
			updateBootImgStatus();
	}
}

function onUploadStart(evt) {
	alert("Started");
}

function onUploadProgress(evt) {
	var progressBar = document.getElementById("upld_prgrs");

	if (evt.lengthComputable) {
		if (progressBar.value > 0) {
			document.getElementById('upld_status').value = "Uploading . . . . .";
		}
		progressBar.max = evt.total;
		progressBar.value = evt.loaded;

	}
}

function onUploadSuccess(evt) {
	initiateCrcValidation();
	document.getElementById('upld_status').value = "Verifying CRC32 . . . . .";
}

function onUploadFailed(evt) {
	var imgId = 'A';
	var imgFile = document.getElementById("img_file").files[0];

	if (document.getElementById("recBimg").checked)
		imgId = 'B';
	else if (document.getElementById("recWICimg").checked)
		imgId = "WIC"

	document.getElementById('upld_status').value = "Upload Failed . . . . .";
	alert("Failed to update image " + imgId);
	enableAllUsrInputs();
}

function onUploadCanceled(evt) {
	var imgId = 'A';
	var imgFile = document.getElementById("img_file").files[0];

	if (document.getElementById("recBimg").checked)
		imgId = 'B';
	else if (document.getElementById("recWICimg").checked)
		imgId = "WIC"

	document.getElementById('upld_status').value = "Upload Canceled . . . . .";
	alert("Canceled update image " + imgId + " operation");
	enableAllUsrInputs();
}

function disableAllUsrInputs() {
	document.getElementById("brws_btn").disabled = true;
	document.getElementById("upld_btn").disabled = true;
	document.getElementById("sbmt_btn").disabled = true;
	document.getElementById("recAimg").disabled = true;
	document.getElementById("recBimg").disabled = true;
	document.getElementById("recWICimg").disabled = true;
}

function enableAllUsrInputs() {
	document.getElementById("brws_btn").disabled = false;
	document.getElementById("upld_btn").disabled = false;
	document.getElementById("sbmt_btn").disabled = false;
	document.getElementById("recAimg").disabled = false;
	document.getElementById("recBimg").disabled = false;
	document.getElementById("recWICimg").disabled = false;
}

function initiateImgUpload () {
	var imgId = "A";
	var imgFile = document.getElementById("img_file").files[0];

	if (document.getElementById("recBimg").checked)
		imgId = "B";
	else if (document.getElementById("recWICimg").checked)
		imgId = "WIC"

	var url = '/download_img' + imgId;
	var xhr = new XMLHttpRequest();
	var fd = new FormData();
	fd.append(url, imgFile);
	document.getElementById("upld_prgrs").style.visibility = "visible";
	document.getElementById('upld_status').value = "Erasing Flash . . . . .";
	document.getElementById("upld_prgrs").value = 0;
	xhr.upload.addEventListener("progress", onUploadProgress, false);
	xhr.addEventListener("load", onUploadSuccess, false);
	xhr.addEventListener("error", onUploadFailed, false);
	xhr.addEventListener("abort", onUploadCanceled, false);
	xhr.open("POST", url, true);
	xhr.send(fd);
}

function onUpload() {
	var imgId = "A";
	var imgFile = document.getElementById("img_file").files[0];

	if (document.getElementById("recBimg").checked)
		imgId = "B";
	else if (document.getElementById("recWICimg").checked)
		imgId = "WIC"

	var progressBar = document.getElementById("upld_prgrs");
	progressBar.value = 0;

	var imgFile = document.getElementById("img_file").files[0];
	extension = imgFile.name.split('.').pop() + '';
	if (((imgId == "A") || (imgId == "B")) && (extension.toUpperCase() != "BIN")) {
		alert("Invalid file type for image " + imgId + ". File should be of .bin type.");
	}
	else if ((imgId == "WIC") && (extension.toUpperCase() != "WIC")) {
		alert("Invalid file type for image " + imgId + ". File should be of .wic type.");
	}
	else {

		if (confirm("Are you sure you want to update image " + imgId + "?")) {
			disableAllUsrInputs();
			document.getElementById("upld_status").style.visibility = "visible";
			document.getElementById('upld_status').value = "Calculating CRC32 . . . . .";
			document.getElementById("upld_prgrs").style.visibility = "visible";
			startCalcCrc32(imgFile);
		}
	}
}

function onBrws() {
	var imgFile = document.getElementById('img_file')
	imgFile.onchange = e => {
		var file = e.target.files[0];
		if (file) {
			document.getElementById("upld_btn").disabled = false;
			document.getElementById("fileName").setAttribute("fd", file);
			document.getElementById("fileName").value = file.name;
			var fileSize = 0;
			if (file.size >= 1073741824)
					fileSize = (Math.round(file.size * 100 / 1073741824) / 100).toString() + ' GB';
			else if (file.size >= 1048576)
					fileSize = (Math.round(file.size * 100 / 1048576) / 100).toString() + ' MB';
			else if (file.size >= 1024)
					fileSize = (Math.round(file.size * 100 / 1024) / 100).toString() + ' Kb';
			else
					fileSize = file.size + 'bytes';

			var divfileSize = document.getElementById('fileSize');
			divfileSize.value = fileSize;
			document.getElementById("upld_prgrs").value = 0;
			document.getElementById("upld_prgrs").style.visibility="hidden";
			document.getElementById("upld_status").style.visibility = "hidden";
		}
	}
	imgFile.click();
}

function flashEraseStatus(imgId) {
	var xhr = new XMLHttpRequest();
	xhr.open("GET", "flash_erase_status", true);
	xhr.send();
	xhr.onload = function() {
		var obj = JSON.parse(this.responseText);
		var progress = parseInt(obj.Progress);
		document.getElementById("upld_prgrs").value = progress;
		if (progress < 100)
			flashEraseStatus();
		else if (progress >= 100) {
			const event = new CustomEvent('FlashEraseDone', { detail: imgId});
			document.getElementById("upld_btn").dispatchEvent(event);
		}
	}
}

function flashErase() {
	var imgId = 'A';
	var imgFile = document.getElementById("img_file").files[0];

	if (document.getElementById("recBimg").checked)
		imgId = 'B';
	else if (document.getElementById("recWICimg").checked)
		imgId = "WIC"

	var xhr = new XMLHttpRequest();
	xhr.open("GET", "flash_erase_img" + imgId, true);
	document.getElementById("upld_prgrs").style.visibility = "visible";
	document.getElementById('upld_status').value = "Erasing Flash . . . . .";
	document.getElementById("upld_prgrs").value = 0;
	document.getElementById("upld_prgrs").max = 100;
	xhr.send();
	xhr.onload = flashEraseStatus(imgId);
}

function onCrcComplete(evt) {
	ImgCrc = parseInt(evt.detail);
	flashErase();
}

function startCalcCrc32(file) {
	var crc = 0xFFFFFFFF;
	var fileSize   = file.size;
	var offset = 0;
	var chunkSize = 64 * 1024;

	function onLoadHandler(evt) {
		if (evt.target.error == null) {
			offset += evt.target.result.byteLength;
			feedData2Crc32Engine(evt.target.result)
		} else {
			alert("ERROR: File read failed during CRC32 calculation");
			return;
		}

		var progressBar = document.getElementById("upld_prgrs");
		if (offset >= fileSize) {
			crc = (crc ^ (-1)) >>> 0;
			progressBar.max = 0;
			const event = new CustomEvent('CrcDone', { detail: crc });
			document.getElementById("upld_btn").dispatchEvent(event);
		}
		else {
			progressBar.max = fileSize;
			progressBar.value = offset;
			readFileChunk(offset, chunkSize, file);
		}
	}

	function readFileChunk (_offset, length, _file) {
		var frd = new FileReader();
		var dataBlob = _file.slice(_offset, length + _offset);
		frd.onload = onLoadHandler;
		frd.readAsArrayBuffer(dataBlob);
	}

	function buildCrc32Table (){
		var n;
		var crcTable = [];

		for(var i = 0; i < 256; i++){
			n = i;
			for(var j = 0; j < 8; j++){
				n = ((n & 1) ? (0xEDB88320 ^ (n >>> 1)) : (n >>> 1));
			}
			crcTable[i] = n;
		}

		return crcTable;
	}

	function feedData2Crc32Engine (data) {
		var buf = new Int8Array(data)
		for (var i = 0; i < data.byteLength; i++ ) {
			crc = (crc >>> 8) ^ crcTable[(crc ^ buf[i]) & 0xFF];
		}
	};

	var crcTable = buildCrc32Table();
    readFileChunk(offset, chunkSize, file);
}

function initiateCrcValidation() {
	var obj = { crc: ImgCrc }
	var http = new XMLHttpRequest();
	http.open("POST", "validate_crc", true);
	var params = JSON.stringify(obj);
	http.send(params);

	http.onload = function() {
		var obj = JSON.parse(this.responseText);
		var imgId = 'A';
		var imgFile = document.getElementById("img_file").files[0];

		if (document.getElementById("recBimg").checked)
			imgId = 'B';
		else if (document.getElementById("recWICimg").checked)
			imgId = "WIC"

		if(obj.Status == "Success") {
			document.getElementById('upld_status').value = "Upload successful . . . . .";
			alert("Successfully updated image " + imgId);
		}
		else {
			document.getElementById('upld_status').value = "Upload failed . . . . .";
			alert("CRC check failed after downloading image " + imgId);
		}

		updateBootImgStatus();
		enableAllUsrInputs();
	}
}
