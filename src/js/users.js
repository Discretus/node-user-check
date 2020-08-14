const usersAddon = require('bindings')('users');

/**
 * Checks if user exists
 * @param {string} name - User name
 * @return {boolean}
 */
function userExists(name) {
  return usersAddon.checkUserExists(name);
}

module.exports = {
  userExists,
};
