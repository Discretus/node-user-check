const express = require('express');
const path = require('path');
const { userExists } = require('./src/js/users');

const app = express();
const port = 4000;

const htmlPath = path.join(__dirname, 'src', 'html');

app.use(
  express.urlencoded({
    extended: true,
  })
);

app.get('/', (_, res) => {
  res.sendFile(path.join(htmlPath, 'index.html'));
});

app.get('/found', (_, res) => {
  res.sendFile(path.join(htmlPath, 'found.html'));
});

app.get('/notfound', (_, res) => {
  res.sendFile(path.join(htmlPath, 'notFound.html'));
});

app.post('/', (req, res) => {
  if (userExists(req.body.username)) {
    res.redirect('/found');
  } else {
    res.redirect('/notfound');
  }
});

app.listen(port, () => {
  console.log(`App listening at http://localhost:${port}`);
});
