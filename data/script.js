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

function downloadHistoryButton() {

    xhttp = new XMLHttpRequest();
    xhttp.open("GET", `${window.location.protocol}//${window.location.hostname}/historyDepth`, false);
    xhttp.send();

    if (xhttp.readyState == 4 && xhttp.status == 200) {
        const obj = JSON.parse(xhttp.responseText)
        let recordsCount = Number(obj.historyDepth);

        xhttp.open("GET", `${window.location.protocol}//${window.location.hostname}/history?count=${recordsCount}&offset=0`, false);
        xhttp.send();

        if (xhttp.readyState == 4 && xhttp.status == 200) {
            // Create a Blob with the CSV data and type
            const blob = new Blob([["Date,Sonde1 (°C),Sonde2 (°C),Chauffe", xhttp.responseText].join('\n')], { type: 'text/csv' });

            // Create a URL for the Blob
            const url = URL.createObjectURL(blob);

            // Create an anchor tag for downloading
            const a = document.createElement('a');

            // Set the URL and download attribute of the anchor tag
            a.href = url;
            a.download = 'download.csv';

            // Trigger the download by clicking the anchor tag
            a.click();
        }
    }
}
