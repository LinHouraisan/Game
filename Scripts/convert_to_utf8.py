import os

def convert_file_to_utf8(file_path):
    """Convert a single file to UTF-8 encoding (with BOM)"""
    try:
        # Read file content in binary mode
        with open(file_path, 'rb') as f:
            content = f.read()
        
        # Try to decode as UTF-8, if failed, use GBK decoding
        try:
            content.decode('utf-8')
        except UnicodeDecodeError:
            content = content.decode('gbk').encode('utf-8')
        
        # Add UTF-8 BOM (0xEF 0xBB 0xBF)
        bom = b'\xef\xbb\xbf'
        content = bom + content
        
        # Write to file in binary mode
        with open(file_path, 'wb') as f:
            f.write(content)
        print(f"Converted: {file_path}")
    except Exception as e:
        print(f"Failed to convert {file_path}: {e}")

def convert_directory_to_utf8(directory):
    """Recursively traverse the directory, convert all code files to UTF-8 encoding (with BOM)"""
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(('.h', '.cpp', '.c', '.hpp', '.cc', '.cxx', '.hxx')):
                file_path = os.path.join(root, file)
                convert_file_to_utf8(file_path)

if __name__ == "__main__":
    # Set project root directory
    project_root = os.path.dirname(os.path.abspath(__file__))
    print(f"Starting conversion in directory: {project_root}")
    
    # Start conversion
    convert_directory_to_utf8(project_root)
    print("Conversion completed!")