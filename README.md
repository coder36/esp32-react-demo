# Aim
A platform, on which to build ESP32 apps, backed by a React based web app.


## Building

Install [platformIO](https://platformio.org/)


#### Compile the web app

```
cd web
npm i
npm run build
npm run crush    
```

`npm crush` copies and encodes the compiled react project, into the SPIFFS data foleder.  SPIFFS only supports files of 30 characters or less.  `npm crush` encodes each of the filename, into a 30 character filename:

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