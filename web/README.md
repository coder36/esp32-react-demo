# React web development

Assumung you're working on a mac, install [nvm](https://github.com/creationix/nvm):

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