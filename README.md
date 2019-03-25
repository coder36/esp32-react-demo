# ESP32 micro controller
Develop a platform, on which to build ESP32 apps, backed by a React based web app.

Provide API to:
* Scan for wifis
* Set wifi ssid and password
* Provide multiple ssid/passwords
* Get wifi status
* Get esp32 info, eg number of cores
* Turn LED on/off


Demonstrate use of GET, POST, parsing json bodies, sha-256 hardware hashing.  Demonstrate how to integrate a [React](https://reactjs.org/) web application

Libraries:
* Built on top of Arduino core
* [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
* [platformIO](https://platformio.org/) to manage dependecies, flashing etc.  This is really nice plugin and IDE for [vscode](https://code.visualstudio.com/)


![screenshot](https://github.com/coder36/https://github.com/coder36/esp32-react-demo/blob/master/react-screenshot.png)

### API

Once,  the esp32 hs booted, it will create an unsecured local Access point `esp32`, which listens on `192.168.4.1`:


#### List wifi accesspoints:

```
curl http://192.168.4.1/wifi_scan
```

#### Set wifi passwords:
```
curl -v -d '{"wifiList": [ {"ssid" :"LOTHAR", "password" :"secret"}, {"ssid": "BTHub5-XYAB", "secret": "b85b72c9a5"} ]}' http://192.168.4.1/set_wifi
```
The configuration is saved to the file `/esp32config.json`. The esp32, will select the strongest signal then connect to that ssid.  

#### Show wifi status:
```
curl http://192.168.4.1/wifi
```

#### Read back the configuration data from SPIFFS:
```
curl http://192.168.4.1/read?file=/esp32config.json
```

### Show esp32 info, eg number of cores, flash size
```
curl http://192.168.4.1/info
```


### Web app
Navigate to [http://192.168.4.1](http://192.168.4.1/)



# Building


#### Compile the web app

Assumung you're working on a mac, install [nvm](https://github.com/creationix/nvm):

```
curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.34.0/install.sh | bash
nvm install 10
nvm use 10
```

Build the SPIFFS image:

```
cd web
npm i
npm run build
npm run crush    
```

#### Upload the SPIFFs image
On PlatformIO, press control+option-T, then select `PlatformIO: Upload File System image`.  This will upload the web application, onto the SPIFFS partition - see `partition.csv`.

#### Flash the application
Flash the application image by pressing press control+option-T, then select `PlatformIO: Upload`.


Once the ESP32 is flashed, it will create a wifi access point named `esp32`.  This is unsecured, so you can connect directly to it.


# React web development
The react app is based on [create-react-app](https://facebook.github.io/create-react-app/).  Assumung you're working on a mac, you'll need to install [nvm](https://github.com/creationix/nvm):

```
curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.34.0/install.sh | bash
nvm install 10
nvm use 10
```

### Starting the dev server
```
cd web
npm i
npm start
```

Navigate to (http://localhost:3000)[http://localhost:3000]


### Compile for deplopyment onto esp32
```
cd web
npm run build
npm run crush
```

### Notes

`npm crush` copies and encodes the compiled react project files, into the SPIFFS data folder.  SPIFFS only supports files of 30 characters or less.  so `npm crush` encodes each of the filename, into a 30 character filename.  Internally the esp32, converts the incoming URL into the correspoding hash, then reads that from the SPIFFs file systm:

```
6246ebf638ad774803103332c525e7 => /asset-manifest.json
213456c5dc963e03ec1f27600c46c9 => /index.html
b18036488649e7cc8a55b0a02c8b73 => /favicon.ico
d5ead976ced23841402a62764875fd => /manifest.json
da4e23edaca2eb1a813d1387ca5be2 => /precache-manifest.4e32bf94628c72e686ca518952a52968.js
a4f2ba4a583ae099112b3e69bd6173 => /service-worker.js
6b9f611f722e0ed57686b8339f5b13 => /static/css/main.0e7f34d7.chunk.css
43d6ec1b1195ec04669cf17d1f170d => /static/css/main.0e7f34d7.chunk.css.map
b253026378b6df2d286aa5f99cab60 => /static/js/2.8f71a45e.chunk.js
d396e0c052404a2b8c4dd662848056 => /static/js/2.8f71a45e.chunk.js.map
48a1a0c5eb55aa7a702dfaa108fa90 => /static/js/main.72b8790b.chunk.js
b8e85f6a1353b65a467474c643db5e => /static/js/main.72b8790b.chunk.js.map
396bd4c43300d3d93fe10022bb46e4 => /static/js/runtime~main.a8a9905a.js
ed88d6a0748bd951b6350e4906eb4c => /static/js/runtime~main.a8a9905a.js.map
6142f652e489cc82f908922f3772b4 => /static/media/logo.5d5d9eef.svg
```

### Next steps

* Update the react web application, to enable wifi setup, monitoring etc.  