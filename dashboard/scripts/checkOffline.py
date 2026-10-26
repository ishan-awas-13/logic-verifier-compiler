import os
import re

dist_dir = r'C:\Users\thaku\.gemini\antigravity\scratch\dashboard\dist'
found_urls = []

for root, dirs, files in os.walk(dist_dir):
    for f in files:
        if f.endswith(('.js', '.css', '.html')):
            p = os.path.join(root, f)
            with open(p, 'rb') as fp:
                content = fp.read().decode('utf-8', errors='ignore')
                urls = re.findall(r'https?://[^\s"\'<>)]+', content)
                for u in urls:
                    if '127.0.0.1' not in u and 'localhost' not in u:
                        found_urls.append((f, u))

print(f'External URLs count: {len(found_urls)}')
for f, u in found_urls:
    print(f'  {f}: {u}')

