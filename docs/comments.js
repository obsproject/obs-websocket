const fs = require('fs');
const glob = require('glob');
const parseComments = require('parse-comments');

/**
 * Read each file and call `parse-comments` on it.
 *
 * @param  {String|Array} `files` List of file paths to read from.
 * @return {Object|Array}         Array of `parse-comments` objects.
 */
const parseFiles = files => {
    let response = [];
    files.forEach(file => {
        const f = fs.readFileSync(file, 'utf8').toString();
        response = response.concat(parseComments(f));
    });

    return response;
};

/**
 * Filters/sorts the results from `parse-comments`.
 * @param  {Object|Array} `comments` Array of `parse-comments` objects.
 * @return {Object}                  Filtered comments sorted by `@api` and `@category`.
 */
const processComments = comments => {
    let sorted = {};

    comments.forEach(comment => {
        if (typeof comment.api === 'undefined') return;

        // Store the object based on its api (ie. requests, events) and category (ie. general, scenes, etc).
        comment.category = comment.category || 'miscellaneous';

        // Remove some unnecessary properties to avoid result differences in travis.
        comment.comment = undefined;
        comment.context = undefined;

        // Create an entry in sorted for the api/category if one does not exist.
        sorted[comment.api] = sorted[comment.api] || {};
        sorted[comment.api][comment.category] = sorted[comment.api][comment.category] || [];

        // Store the comment in the appropriate api/category.
        sorted[comment.api][comment.category].push(comment);
    });

    return sorted;
};

const files = glob.sync("./../*.@(cpp|h)");
const comments = processComments(parseFiles(files));
fs.writeFileSync('./generated/comments.json', JSON.stringify(comments, null, 2));
