import re

with open('updated-physics-maybe-first-wound-markup.txt', 'r', encoding='utf-8') as f:
    content = f.read()

# Split by file markers: "filename" followed by ---------
# The pattern is: "filename"\n---------\ncontent
pattern = r'"([^"]+)"\s*---------\s*(.*?)(?="[^"]+"\s*---------|$)'
matches = re.findall(pattern, content, re.DOTALL)

for filename, file_content in matches:
    # Clean up the content - remove leading/trailing whitespace
    file_content = file_content.strip()
    # Write to file
    with open(filename, 'w', encoding='utf-8', newline='\n') as f:
        f.write(file_content)
        if not file_content.endswith('\n'):
            f.write('\n')
    print(f"Written: {filename} ({len(file_content)} chars)")

print(f"Total files extracted: {len(matches)}")
