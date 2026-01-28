import base64
import os

files = ["StartChessGame.py", "printQuadrants.py"]
output_script = "deploy_updates.py"

with open(output_script, "w") as out:
    out.write("import base64\nimport os\n\n")
    out.write("def write_file(name, b64_content):\n")
    out.write("    with open(name, 'wb') as f:\n")
    out.write("        f.write(base64.b64decode(b64_content))\n")
    out.write("    print(f'Updated {name}')\n\n")
    
    for filename in files:
        if os.path.exists(filename):
            with open(filename, "rb") as f:
                content = f.read()
                b64 = base64.b64encode(content).decode('utf-8')
                out.write(f"# File: {filename}\n")
                out.write(f"write_file('{filename}', '{b64}')\n\n")
        else:
            print(f"Warning: {filename} not found")
            
print(f"Generated {output_script}")
