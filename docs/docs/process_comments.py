import os
import sys
import json

# The comments parser will return a string type instead of an array if there is only one field
def field_to_array(field):
    if type(field) == str:
        return [field]
    return field

# This raw JSON can be really damn unpredictable. Let's handle that
def field_to_string(field):
    if type(field) == list:
        return field_to_string(field[0])
    elif type(field) == dict:
        return field_to_string(field['description'])
    return str(field)

# Make sure that everything we expect is there
def validate_fields(data, fields):
    for field in fields:
        if field not in data:
            print('WARNING: Missing required item: {}'.format(field))
            return False
    return True

def get_components(data):
    ret = []
    components_raw = data.split('|')
    for component in components_raw:
        ret.append(component.strip())
    return ret

def get_request_fields(fields):
    fields = field_to_array(fields)
    ret = []
    for field in fields:
        components = get_components(field)
        field_out = {}
        field_out['valueName'] = components[0]
        field_out['valueType'] = components[1]
        field_out['valueDescription'] = components[2]
        valueOptionalOffset = 3
        if field_out['valueType'].lower() == 'number':
            valueOptionalOffset += 1
            field_out['valueRestrictions'] = components[3] if components[3] != 'None' else None
        else:
            field_out['valueRestrictions'] = None
        if len(components) <= valueOptionalOffset:
            field_out['valueOptional'] = False
            field_out['valueOptionalBehavior'] = None
        else:
            field_out['valueOptional'] = True
            field_out['valueOptionalBehavior'] = components[valueOptionalOffset]
        ret.append(field_out)
    return ret

def get_response_fields(fields):
    fields = field_to_array(fields)
    ret = []
    for field in fields:
        components = get_components(field)
        field_out = {}
        field_out['valueName'] = components[0]
        field_out['valueType'] = components[1]
        field_out['valueDescription'] = components[2]
        ret.append(field_out)
    return ret

# Open the raw comments output file
with open('../work/comments.json', 'r') as f:
    comments_raw = json.load(f)

# Prepare output variables
enums = []
requests = []
events = []

# Process the raw comments
for comment in comments_raw:
    # Skip unrelated comments like #include
    if 'api' not in comment:
        continue

    api = comment['api']
    if api == 'enums':
        pass
    elif api == 'requests':
        if not validate_fields(comment, ['description', 'requestType', 'complexity', 'rpcVersion', 'initialVersion', 'category']):
            print('WARNING: Failed to process request comment:\n{}'.format(comment))
            continue
        req = {}
        req['description'] = field_to_string(comment.get('lead', '')) + field_to_string(comment['description'])
        req['requestType'] = field_to_string(comment['requestType'])
        req['complexity'] = field_to_string(int(comment['complexity']))
        req['rpcVersion'] = field_to_string(int(comment['rpcVersion']))
        req['initialVersion'] = field_to_string(comment['initialVersion'])
        req['category'] = field_to_string(comment['category'])

        if 'requestField' in comment:
            req['requestFields'] = get_request_fields(comment['requestField'])

        if 'responseField' in comment:
            req['responseFields'] = get_response_fields(comment['responseField'])

        requests.append(req)
    elif api == 'events':
        if not validate_fields(comment, ['description', 'eventType', 'complexity', 'rpcVersion', 'initialVersion', 'category']):
            print('WARNING: Failed to process event comment:\n{}'.format(comment))
            continue
        eve = {}
        eve['description'] = field_to_string(comment.get('lead', '')) + field_to_string(comment['description'])
        eve['eventType'] = field_to_string(comment['eventType'])
        eve['complexity'] = field_to_string(int(comment['complexity']))
        eve['rpcVersion'] = field_to_string(int(comment['rpcVersion']))
        eve['initialVersion'] = field_to_string(comment['initialVersion'])
        eve['category'] = field_to_string(comment['category'])

        if 'dataField' in comment:
            eve['dataFields'] = get_response_fields(comment['dataField'])

        events.append(eve)

finalObject = {'enums': enums, 'requests': requests, 'events': events}

with open('../generated/protocol.json', 'w') as f:
    json.dump(finalObject, f, indent=2)
