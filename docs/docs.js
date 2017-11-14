const fs = require('fs');
const path = require('path');
const toc = require('markdown-toc');
const handlebars = require('handlebars');
const config = require('./config.json');

const helpers = require('handlebars-helpers')({
    handlebars: handlebars
});

// Allows pipe characters to be used within markdown tables.
handlebars.registerHelper('depipe', (text) => {
    return typeof text === 'string' ? text.replace('|', '\\|') : text;
});

const insertHeader = (text) => {
    return '<!-- This file was generated based on handlebars templates. Do not edit directly! -->\n\n' + text;
};

/**
 * Writes `protocol.md` using `protocol.mustache`.
 *
 * @param  {Object} `data` Data to assign to the mustache template.
 */
const generateProtocol = (templatePath, data) => {
    const template = fs.readFileSync(templatePath).toString();
    const generated = handlebars.compile(template)(data);
    return insertHeader(toc.insert(generated));
};

if (!fs.existsSync(config.outDirectory)){
    fs.mkdirSync(config.outDirectory);
}

const comments = fs.readFileSync(path.join(config.outDirectory, 'comments.json'), 'utf8');
const markdown = generateProtocol(config.srcTemplate, JSON.parse(comments));
fs.writeFileSync(path.join(config.outDirectory, 'protocol.md'), markdown);
