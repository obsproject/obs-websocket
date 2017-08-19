const fs = require('fs');
const toc = require('markdown-toc');
const handlebars = require('handlebars');

const helpers = require('handlebars-helpers')({
  handlebars: handlebars
});

// Allows pipe characters to be used within markdown tables.
handlebars.registerHelper('depipe', (text) => {
    return text.replace('|', `\\|`);
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

const comments = fs.readFileSync('./generated/comments.json', 'utf8');
const markdown = generateProtocol('./protocol.hbs', JSON.parse(comments));
fs.writeFileSync('./generated/protocol.md', markdown);
