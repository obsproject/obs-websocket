const fs = require('fs');
const path = require('path');
const glob = require('glob');
const parseComments = require('parse-comments');
const config = require('./config.json');

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
    let errors = [];

    comments.forEach(comment => {
        if (comment.typedef) {
            comment.comment = undefined;
            comment.context = undefined;
            sorted['typedefs'] = sorted['typedefs'] || [];
            sorted['typedefs'].push(comment);
            return;
        }

        if (typeof comment.api === 'undefined') return;
        let validationFailures = validateComment(comment);

        if (validationFailures) {
            errors.push(validationFailures);
        }

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

    if (errors.length) {
        throw JSON.stringify(errors, null, 2);
    }

    return sorted;
};

// Rudimentary validation of documentation content, returns an error object or undefined.
const validateComment = comment => {
    let errors = [];
    [].concat(comment.params).concat(comment.returns).filter(Boolean).forEach(param => {
        if (typeof param.name !== 'string' || param.name === '') {
            errors.push({
                description: `Invalid param or return value name`,
                param: param
            });
        }

        if (typeof param.type !== 'string' || param.type === '') {
            errors.push({
                description: `Invalid param or return value type`,
                param: param
            });
        }
    });

    if (errors.length) {
        return {
            errors: errors,
            fullContext: Object.assign({}, comment)
        };
    }
};

const files = glob.sync(config.srcGlob);
const comments = processComments(parseFiles(files));

if (!fs.existsSync(config.outDirectory)){
    fs.mkdirSync(config.outDirectory);
}

fs.writeFileSync(path.join(config.outDirectory, 'comments.json'), JSON.stringify(comments, null, 2));
