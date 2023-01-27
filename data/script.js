function immediateMonitoringButton() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "immediateMonitoring", true);
    xhttp.send();
    window.location.href = "immediateMonitoring.html";
}

function historizedMonitoringButton() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "historizedMonitoring", true);
    xhttp.send();
}

function gotoSleepButton() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "gotoSleep", true);
    xhttp.send();
}

function backHomeButton() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "", true);
    xhttp.send();
}

function setTimeButton() {
    const now = new Date();
    console.log(now.getTime() / 1000);
    console.log(now.getTimezoneOffset());
    const utcSecondsSinceEpoch = Math.round(now.getTime() / 1000);

    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "setTime?timeEpoch=" + utcSecondsSinceEpoch.toString());
    xhttp.send();
}

function clearHistoryButton() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "clearHistory", true);
    xhttp.send();    
}

function downloadFile(urlToSend) {
    var req = new XMLHttpRequest();
    req.open("GET", urlToSend, true);
    req.responseType = "blob";
    req.onload = function (event) {
        var blob = req.response;
        var fileName = req.getResponseHeader("fileName") //if you have the fileName header available
        var link=document.createElement('a');
        link.href=window.URL.createObjectURL(blob);
        link.download=fileName;
        link.click();
    };

    req.send();
}

$('#GetFile').on('click', function () {
    $.ajax({
        url: 'http://192.168.4.22/EcsMonitoring.tsv',
        method: 'GET',
        xhrFields: {
            responseType: 'blob'
        },
        success: function (data) {
            var a = document.createElement('a');
            var url = window.URL.createObjectURL(data);
            a.href = url;
            a.download = 'EcsMonitoring.tsv';
            document.body.append(a);
            a.click();
            a.remove();
            window.URL.revokeObjectURL(url);
        }
    });
});

document.querySelector('#form').addEventListener('submit', (e)=>{
    e.preventDefault();
    let xhr = new XMLHttpRequest();
    //set the request type to post and the destination url to '/convert'
    xhr.open('POST', 'downloadHistory');
    //set the reponse type to blob since that's what we're expecting back
    xhr.responseType = 'blob';
    let formData = new FormData(this);
    xhr.send(formData); 
  
    xhr.onload = function(e) {
        if (this.status == 200) {
            // Create a new Blob object using the response data of the onload object
            var blob = new Blob([this.response], {type: 'image/pdf'});
            //Create a link element, hide it, direct it towards the blob, and then 'click' it programatically
            let a = document.createElement("a");
            a.style = "display: none";
            document.body.appendChild(a);
            //Create a DOMString representing the blob and point the link element towards it
            let url = window.URL.createObjectURL(blob);
            a.href = url;
            a.download = 'myFile.pdf';
            //programatically click the link to trigger the download
            a.click();
            //release the reference to the file by revoking the Object URL
            window.URL.revokeObjectURL(url);
        }else{
            //deal with your error state here
        }
    };
  });