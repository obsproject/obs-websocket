import logging
logging.basicConfig(level=logging.INFO, format="%(asctime)s [%(levelname)s] %(message)s")
import os
import sys
import json

enumTypeOrder = [
    'WebSocketOpCode',
    'WebSocketCloseCode',
    'RequestBatchExecutionType',
    'RequestStatus',
    'EventSubscription'
]

categoryOrder = [
    'General',
    'Config',
    'Sources',
    'Scenes',
    'Inputs',
    'Transitions',
    'Filters',
    'Scene Items',
    'Outputs',
    'Stream',
    'Record',
    'Media Inputs',
    'High-Volume'
]

#######################################################################################################################

def read_file(fileName):
    with open(fileName, 'r') as f:
        return f.read()

def get_category_items(items, category):
    ret = []
    for item in requests:
        if item['category'] != category:
            continue
        ret.append(item)
    return ret

def get_enums_toc(enums):
    ret = ''
    for enumType in enumTypeOrder:
        enum = None
        for enumIt in enums:
            if enumIt['enumType'] == enumType:
                enum = enumIt
                break
        if not enum:
            continue
        rawType = enumType.replace(' ', '-').lower()
        ret += '- [{}](#{})\n'.format(enumType, rawType)
        for enumIdentifier in enum['enumIdentifiers']:
            enumIdentifierString = enumIdentifier['enumIdentifier']
            ret += '  - [{}](#{}-{})\n'.format(enumIdentifierString, rawType, enumIdentifierString.replace(' ', '-').lower())
    return ret

def get_requests_toc(requests):
    ret = ''
    for category in categoryOrder:
        requestsOut = []
        for request in requests:
            if request['category'] != category.lower():
                continue
            requestsOut.append(request)
        if not len(requestsOut):
            continue
        rawCategory = category.replace(' ', '-').lower()
        ret += '- [{}](#{})\n'.format(category, rawCategory)
        for request in requestsOut:
            requestType = request['requestType']
            ret += '  - [{}](#{}-{})\n'.format(requestType, rawCategory, requestType.replace(' ', '-').lower())
    return ret

def get_events_toc(events):
    ret = ''
    for category in categoryOrder:
        eventsOut = []
        for event in events:
            if event['category'] != category.lower():
                continue
            eventsOut.append(event)
        if not len(eventsOut):
            continue
        rawCategory = category.replace(' ', '-').lower()
        ret += '- [{}](#{})\n'.format(category, rawCategory)
        for event in eventsOut:
            requestType = event['eventType']
            ret += '  - [{}](#{}-{})\n'.format(requestType, rawCategory, requestType.replace(' ', '-').lower())
    return ret

#######################################################################################################################

with open('../generated/protocol.json', 'r') as f:
    protocol = json.load(f)

output = "<!-- This file was automatically generated. Do not edit directly! -->\n"

output += read_file('partials/introduction.md')

output += '\n\n'

output += read_file('partials/enumsHeader.md')
output += get_enums_toc(protocol['enums'])

output += '\n\n'

output += read_file('partials/requestsHeader.md')
output += get_requests_toc(protocol['requests'])

output += '\n\n'

output += read_file('partials/eventsHeader.md')
output += get_events_toc(protocol['events'])

output += '\n\n'

with open('../generated/protocol.md', 'w') as f:
    f.write(output)
