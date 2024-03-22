// web_page.h
#ifndef WEB_PAGE_H
#define WEB_PAGE_H

const char AP_HTML[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
        <title>Configure Wi-Fi</title>
        <style>
            body, html {
                height: 100%;
                margin: 0;
                font-family: Arial, sans-serif;
                display: flex;
                justify-content: center;
                align-items: center;
                background-color: #000;
            }
            .container {
                background-color: #fff;
                border-radius: 10px;
                box-shadow: 0px 0px 20px rgba(0, 0, 0, 0.5);
                padding: 20px;
                max-width: 600px;
                width: 100%;
            }
            .card {
                background-color: #fff;
                border-radius: 10px;
                box-shadow: 0px 0px 20px rgba(0, 0, 0, 0.5);
                padding: 40px;
            }
            form {
                text-align: center;
            }
            input[type=text], input[type=password], select {
                width: calc(100% - 22px);
                padding: 10px;
                margin: 8px 0;
                display: inline-block;
                border: 1px solid #ccc;
                box-sizing: border-box;
            }
            input[type=submit] {
                width: 100%;
                background-color: #4CAF50;
                color: white;
                padding: 14px 20px;
                margin: 8px 0;
                border: none;
                cursor: pointer;
                border-radius: 5px;
            }
            input[type=submit]:hover {
                opacity: 0.8;
            }
            a {
                color: dodgerblue;
                text-decoration: none;
            }
            .scan-results {
                margin-top: 20px;
                padding: 20px;
                border: 1px solid #ddd;
                border-radius: 5px;
                background-color: #f9f9f9;
                max-height: 200px;
                overflow-y: auto;
            }
            .scan-results ul {
                list-style-type: none;
                padding: 0;
                margin: 0;
            }
            .loading {
                display: flex;
                justify-content: center;
                align-items: center;
                height: 100px;
            }
            .loading::after {
                content: '';
                width: 20px;
                height: 20px;
                border-radius: 50%;
                border: 2px solid #ccc;
                border-top-color: #333;
                animation: spin 1s linear infinite;
            }
            @keyframes spin {
                to { transform: rotate(360deg); }
            }
        </style>
    </head>
    <body>
        <div class="container">
            <div class="card">
                <h2>Configure Wi-Fi</h2>
                <form action="/connect" method="post">
                    SSID:<br>
                    <select name="ssid" id="ssid">
                        <!-- Dropdown options will be populated dynamically -->
                    </select><br>
                    Password:<br>
                    <input type="password" name="password"><br><br>
                    <input type="submit" value="Connect">
                </form>
            </div>
            <div class="loading" id="loading">
                <!-- Loading animation will be displayed here -->
            </div>
        </div>
        <script>
            document.addEventListener('DOMContentLoaded', function() {
                fetch('/scan')
                .then(response => response.text())
                .then(data => {

                    populateSSIDDropdown(data);
                    document.getElementById('loading').style.display = 'none';
                })
                .catch(error => console.error('Error:', error));
                scanNetworks();
            });

            function populateSSIDDropdown(scanResults) {
                const ssidDropdown = document.getElementById('ssid');
                const scanResultsList = scanResults.split('<li>').slice(1);
                const ssids = scanResultsList.map(result => result.split('(')[0].trim());
                ssids.forEach(ssid => {
                    const option = document.createElement('option');
                    option.value = ssid;
                    option.textContent = ssid;
                    ssidDropdown.appendChild(option);
                });
            }

            function scanNetworks() {
                setInterval(function() {
                    fetch('/scan')
                    .then(response => response.text())
                    .then(data => {
                        populateSSIDDropdown(data);
                    })
                    .catch(error => console.error('Error:', error));
                }, 30000);
            }
        </script>
    </body>
    </html>
)rawliteral";


const char STA_HTML[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Timer Control</title>
        <link href="https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css" rel="stylesheet">
        //particle.js
        <script src="https://cdn.jsdelivr.net/particles.js/2.0.0/particles.min.js"></script>
        <script src="https://cdnjs.cloudflare.com/ajax/libs/three.js/r128/three.min.js"></script>
        <style>
            canvas {
                display: block;
            }
            body, html {
                height: 100%;
                margin: 0;
            }
            #particles-js {
                position: absolute;
                width: 100%;
                height: 100%;
                background-color: #000;
                top: 0;
                left: 0;
                z-index: 0;
            }
            .content {
                position: absolute;
                z-index: 1;
                top: 50%;
                left: 50%;
                transform: translate(-50%, -50%);
                width: 90%;
                max-width: 400px;
                padding: 20px;
                background-color: rgba(255, 255, 255, 0.8);
                border-radius: 10px;
                box-shadow: 0 0 15px rgba(0, 0, 0, 0.2);
            }
            .modeButton {
                transition: background-color 0.3s;
            }
            .modeButton:hover {
                cursor: pointer;
            }
            .presetButton {
                transition: background-color 0.3s;
            }
            .presetButton:hover {
                cursor: pointer;
            }
            #toastContainer {
              position: fixed;
              bottom: 20px;
              right: 20px;
              z-index: 1000;
            }

            .toast {
              background-color: #333;
              color: white;
              padding: 16px;
              margin-top: 16px;
              opacity: 0;
              transition: opacity 0.5s, margin-top 0.5s;
            }
        </style>
    </head>
    <body class="flex flex-col items-center justify-center h-screen">
        <div id="particles-js"></div>
        <div id="toastContainer" class="fixed bottom-0 right-0 m-8"></div>
        <div class="content">
            <h2 class="text-lg font-bold mb-4">Timer Control</h2>
            <button type="button" id="resetButton" class="py-2 px-4 font-semibold rounded-lg shadow-md text-white bg-red-500 hover:bg-red-700 mt-4">Reset WiFi</button>
            <div id="modeSelector" class="mb-4">
                <button id="stopwatchMode" class="modeButton py-2 px-4 font-semibold rounded-lg shadow-md text-white bg-blue-500 hover:bg-green-700">Stopwatch</button>
                <button id="countdownMode" class="modeButton py-2 px-4 font-semibold rounded-lg shadow-md text-white bg-blue-500 hover:bg-blue-700">Countdown</button>
            </div>
            <div id="presets" class="mb-4">
                <button class="presetButton py-2 px-4 font-semibold rounded-lg shadow-md text-white bg-gray-500 hover:bg-gray-700" data-minutes="10" data-seconds="0">10 Min</button>
                <button class="presetButton py-2 px-4 font-semibold rounded-lg shadow-md text-white bg-gray-500 hover:bg-gray-700" data-minutes="5" data-seconds="0">5 Min</button>
                <button class="presetButton py-2 px-4 font-semibold rounded-lg shadow-md text-white bg-gray-500 hover:bg-gray-700" data-minutes="1" data-seconds="0">1 Min</button>
            </div>
            <form id="timeForm" method="POST">
                <input type="number" id="minutes" name="minutes" placeholder="Minutes" min="0" max="99" class="border-2 border-gray-200 rounded p-2 mr-2">
                <input type="number" id="seconds" name="seconds" placeholder="Seconds" min="0" max="59" class="border-2 border-gray-200 rounded p-2">
                <button type="button" id="submitButton" class="py-2 px-4 font-semibold rounded-lg shadow-md text-white bg-red-500 hover:bg-red-700">Start</button>
            </form>
        </div>
        <script>
            let currentMode = '';
            document.querySelectorAll('.modeButton').forEach(button => {
                button.addEventListener('click', function() {
                    currentMode = this.id.replace('Mode', '').toLowerCase();
                    document.getElementById('modeSelector').querySelectorAll('.modeButton').forEach(button => button.classList.remove('bg-green-700', 'bg-blue-700'));
                    this.classList.add('bg-green-700');
                });
            });
            document.querySelectorAll('.presetButton').forEach(button => {
                button.addEventListener('click', function() {
                    document.getElementById('minutes').value = this.dataset.minutes;
                    document.getElementById('seconds').value = this.dataset.seconds;
                    startTimer(); // Call startTimer function on preset click
                });
            });
            document.getElementById('submitButton').addEventListener('click', startTimer);

            function startTimer() {
                var minutes = document.getElementById('minutes').value;
                var seconds = document.getElementById('seconds').value;
                fetch('/timer-action', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded',
                    },
                    body: `action=start&mode=${currentMode}&minutes=${minutes}&seconds=${seconds}`
                })
                .then(response => response.text())
                .then(data => console.log(data))
                .catch(error => console.error('Error:', error));
            }

            document.getElementById('resetButton').addEventListener('click', resetWiFiCredentials);

            function resetWiFiCredentials() {
                fetch('/reset-wifi', {
                    method: 'POST'
                })
                .then(response => response.text())
                .then(data => {
                    console.log("WiFi credentials reset successfully.");
                })
                .catch(error => console.error('Error resetting WiFi credentials:', error));
            }


            // Particle.js
            particlesJS('particles-js',
            {
                "particles": {
                "number": {
                    "value": 50,
                    "density": {
                    "enable": true,
                    "value_area": 800
                    }
                },
                "color": {
                    "value": "#ffffff"
                },
                "shape": {
                    "type": "circle",
                    "stroke": {
                    "width": 0,
                    "color": "#000000"
                    }
                },
                "opacity": {
                    "value": 0.5,
                    "random": false
                },
                "size": {
                    "value": 3,
                    "random": true
                },
                "line_linked": {
                    "enable": true,
                    "distance": 150,
                    "color": "#ffffff",
                    "opacity": 0.4,
                    "width": 1
                },
                "move": {
                    "enable": true,
                    "speed": 6,
                    "direction": "none",
                    "random": false,
                    "straight": false,
                    "out_mode": "out",
                    "bounce": false,
                    "attract": {
                    "enable": false,
                    "rotateX": 600,
                    "rotateY": 1200
                    }
                }
                },
                "interactivity": {
                "detect_on": "canvas",
                "events": {
                    "onhover": {
                    "enable": true,
                    "mode": "repulse"
                    },
                    "onclick": {
                    "enable": true,
                    "mode": "push"
                    },
                    "resize": true
                }
                },
                "retina_detect": true
            }
            );

            document.addEventListener('DOMContentLoaded', function() {
                if ("Notification" in window) {
                    Notification.requestPermission();
                }
            });

            var ws = new WebSocket((window.location.protocol === 'https:' ? 'wss://' : 'ws://') + window.location.hostname + (window.location.port ? ':' + window.location.port : '') + '/ws');

            ws.onmessage = function(event) {
                var data = JSON.parse(event.data);
                console.log('Message from server:', data);

                if(data.type === "statusUpdate") {
                    console.log("Timer mode:", data.mode);
                    console.log("Time elapsed:", data.timeElapsed);
                }
            };
            ws.onopen = function() {
                console.log('WebSocket connection established');
            };


            ws.onclose = function() {
                console.log('WebSocket connection closed');
            };

            ws.onerror = function(error) {
                console.log('WebSocket Error:', error);
            };
        </script>
    </body>
    </html>
)rawliteral";


const char DOCS_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>API Documentation</title>
    <link href="https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css" rel="stylesheet">
    <style>
        .card {
            background: white;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
        }
    </style>
</head>
<body class="bg-gray-100 font-sans leading-normal tracking-normal">

<div class="container mx-auto my-8 max-w-full">
    <h1 class="text-4xl font-bold text-center mb-8">API Documentation</h1>
    <div class="grid gap-6 md:grid-cols-2 lg:grid-cols-3">
        <!-- Endpoints -->
        <div class="card">
            <h2 class="font-bold text-xl mb-2">Endpoints</h2>
            <ul class="list-disc space-y-2 pl-5 text-gray-700">
                <li>
                    <strong>GET /reset-wifi</strong> - Resets the WiFi credentials and restarts the device.
                    <br><a href="#" class="text-blue-500 hover:text-blue-600 dynamic-url" data-path="/reset-wifi">Dynamic URL</a>
                </li>
                <li>
                    <strong>POST /connect</strong> - Connects to a specified WiFi network. Requires parameters: <code>ssid</code> and <code>password</code>.
                    <br><span class="text-gray-600 dynamic-curl" data-path="/connect" data-command="post">Dynamic CURL</span>
                </li>
                <li>
                    <strong>GET /scan</strong> - Scans for available WiFi networks and returns the results.
                    <br><a href="#" class="text-blue-500 hover:text-blue-600 dynamic-url" data-path="/scan">Dynamic URL</a>
                </li>
                <li>
                    <strong>POST /timer-action</strong> - Starts, stops, or pauses the timer with given parameters. Requires parameters: <code>action</code> (start, stop, pause), <code>mode</code> (stopwatch, countdown), <code>minutes</code>, and <code>seconds</code>.
                    <br><span class="text-gray-600 dynamic-curl" data-path="/timer-action" data-command="post">Dynamic CURL</span>
                </li>
                <li>
                    <strong>POST /update-display</strong> - Updates the display with the specified minutes and seconds. Requires parameters: <code>minutes</code> and <code>seconds</code>.
                    <br><span class="text-gray-600 dynamic-curl" data-path="/update-display" data-command="post">Dynamic CURL</span>
                </li>
                <li>
                    <strong>GET /device-info</strong> - Returns information about the device.
                    <br><a href="#" class="text-blue-500 hover:text-blue-600 dynamic-url" data-path="/device-info">Dynamic URL</a>
                </li>
            </ul>
        </div>

        <!-- WebSocket Messages -->
        <div class="card lg:col-span-2">
            <h2 class="font-bold text-xl mb-2">WebSocket Messages</h2>
            <p>Live updates from the server will appear below:</p>
            <div id="wsMessages" class="mt-2 p-2 bg-gray-100 rounded h-32 overflow-auto"></div>
        </div>

        <!-- Interactive Example -->
        <div class="card lg:col-span-3">
            <h2 class="font-bold text-xl mb-2">Interactive Example</h2>
            <p>Use the form below to send a timer action command to the server:</p>
            <form id="timerActionForm" class="space-y-4">
                <input type="text" id="action" name="action" placeholder="Action (start, stop, pause)" class="w-full border p-2 rounded" required>
                <input type="text" id="mode" name="mode" placeholder="Mode (stopwatch, countdown)" class="w-full border p-2 rounded" required>
                <div class="flex gap-4">
                    <input type="number" id="minutes" name="minutes" placeholder="Minutes" class="w-full border p-2 rounded" min="0" required>
                    <input type="number" id="seconds" name="seconds" placeholder="Seconds" class="w-full border p-2 rounded" min="0" required>
                </div>
                <button type="submit" class="w-full bg-blue-500 text-white p-2 rounded hover:bg-blue-600">Send Command</button>
            </form>
        </div>
    </div>
</div>

<script>
    document.addEventListener('DOMContentLoaded', function() {
        var currentHostname = window.location.hostname;
        var protocol = window.location.protocol;
        var port = window.location.port ? ':' + window.location.port : '';
        var baseUrl = protocol + '//' + currentHostname + port;

        // Update URLs dynamically
        document.querySelectorAll('.dynamic-url').forEach(function(element) {
            var path = element.getAttribute('data-path');
            element.href = baseUrl + path;
            element.innerText = baseUrl + path;
        });

        document.querySelectorAll('.dynamic-curl').forEach(function(element) {
            var path = element.getAttribute('data-path');
            var command = element.getAttribute('data-command');
            element.innerText = `curl -X ${command.toUpperCase()} ${baseUrl}${path} -d "ssid=YourSSID&password=YourPassword"`;
        });

        // Form submission handling
        document.getElementById('timerActionForm').addEventListener('submit', function(e) {
            e.preventDefault();
            const action = document.getElementById('action').value;
            const mode = document.getElementById('mode').value;
            const minutes = document.getElementById('minutes').value;
            const seconds = document.getElementById('seconds').value;

            fetch('/timer-action', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: `action=${action}&mode=${mode}&minutes=${minutes}&seconds=${seconds}`
            })
            .then(response => response.text())
            .then(data => alert(data))
            .catch(error => console.error('Error:', error));
        });

        // WebSocket connection
        var ws = new WebSocket((protocol === 'https:' ? 'wss://' : 'ws://') + currentHostname + port + '/ws');

        ws.onmessage = function(event) {
            console.log('Message from server:', event.data);
            try {
                var messageObj = JSON.parse(event.data);
                handleJsonMessage(messageObj);
            } catch (error) {
                handleRawMessage(event.data);
            }
        };
    });

    function handleJsonMessage(messageObj) {
        var messagesDiv = document.getElementById('wsMessages');
        var messageElement = document.createElement('p');
        messageElement.textContent = `[${messageObj.action}] ${messageObj.message}`;
        messagesDiv.appendChild(messageElement);
        messagesDiv.scrollTop = messagesDiv.scrollHeight;
    }

    function handleRawMessage(rawMessage) {
        var messagesDiv = document.getElementById('wsMessages');
        var messageElement = document.createElement('p');
        messageElement.textContent = `Raw Message: ${rawMessage}`;
        messagesDiv.appendChild(messageElement);
        messagesDiv.scrollTop = messagesDiv.scrollHeight;
    }
</script>
</body>
</html>

)rawliteral";

#endif