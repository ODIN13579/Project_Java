#!/usr/bin/env python3
"""Lightweight offline checks; NOT an official full OpenAPI schema validator.
Run: python validate_openapi.py (requires PyYAML)
"""
from pathlib import Path
import yaml
import sys
from collections import Counter

root = Path(__file__).resolve().parent
spec = yaml.safe_load((root / 'openapi.yaml').read_text(encoding='utf-8'))
errors=[]; warnings=[]
if spec.get('openapi') != '3.0.3': errors.append('Expected OpenAPI 3.0.3')
if not spec.get('info',{}).get('title'): errors.append('Missing info.title')
components=spec.get('components',{})

def walk(obj, loc='$'):
    if isinstance(obj,dict):
        if '$ref' in obj:
            target=obj['$ref']
            if not isinstance(target,str) or not target.startswith('#/'):
                errors.append(f'External/unrecognized ref: {loc}: {target}')
            else:
                node=spec
                for key in target[2:].split('/'):
                    key=key.replace('~1','/').replace('~0','~')
                    if not isinstance(node,dict) or key not in node:
                        errors.append(f'Invalid ref at {loc}: {target}')
                        break
                    node=node[key]
        for k,v in obj.items(): walk(v, loc+'.'+str(k))
    elif isinstance(obj,list):
        for i,v in enumerate(obj): walk(v,loc+f'[{i}]')
walk(spec)
ids=[]; operations=0
valid_methods={'get','put','post','patch','delete','options','head','trace'}
for path,item in spec.get('paths',{}).items():
    if not path.startswith('/'):errors.append('Nonabsolute path '+path)
    for method,op in item.items():
        if method not in valid_methods:continue
        operations+=1
        if not op.get('operationId'):errors.append(f'Missing operationId: {method} {path}')
        else:ids.append(op['operationId'])
        if not op.get('responses'):errors.append(f'Missing responses: {method} {path}')
        if 'security' not in op:errors.append(f'Missing security: {method} {path}')
        if not op.get('tags'):errors.append(f'Missing tags: {method} {path}')
        declared={p['name'] for p in op.get('parameters',[]) if p.get('in')=='path'}
        import re
        needed=set(re.findall(r'{([^{}]+)}',path))
        if declared!=needed:errors.append(f'Path parameters mismatch {method} {path}: {declared} != {needed}')
        if method in ('post','put','patch') and 'requestBody' not in op and 'logout' not in op.get('operationId',''):
            warnings.append(f'No request body: {method} {path}')
        if 'x-build-priority' not in op:errors.append(f'Missing priority: {method} {path}')
for key,cnt in Counter(ids).items():
    if cnt>1:errors.append(f'Duplicate operationId: {key}')
print(f'Parsed YAML: {operations} operations on {len(spec.get("paths",{}))} paths; '
      f'{len(components.get("schemas",{}))} schemas; {len(ids)} operationIds')
for w in warnings:print('WARN:',w)
for e in errors:print('ERROR:',e)
if errors:
    print('FAILED: structural checks; use an official OpenAPI 3.0 validator for complete compliance.')
    sys.exit(1)
print('PASS: local YAML, refs, operationIds, parameters, responses and security checks.')
print('NOTE: not a full official OpenAPI 3.0 schema validation or live API test.')
