const users = require('bindings')('users');

console.log(users.getUserNames());

console.log(users.getUserNames().includes('Kiril'));
