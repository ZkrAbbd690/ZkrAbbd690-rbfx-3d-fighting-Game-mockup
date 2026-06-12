# conv-pdf-to-text.py
import pytesseract
from pdf2image import convert_from_path
import os
import sys
import subprocess

def find_tesseract():
    """Find tesseract executable from system PATH"""
    tesseract_path = None
    
    # Try 'where' command on Windows to find tesseract in PATH
    try:
        result = subprocess.run(['where', 'tesseract'], capture_output=True, text=True)
        if result.returncode == 0:
            tesseract_path = result.stdout.strip().split('\n')[0]
            print(f"Found Tesseract at: {tesseract_path}")
    except:
        pass
    
    # If not found, try common installation paths
    if not tesseract_path or not os.path.exists(tesseract_path):
        common_paths = [
            r'C:\Program Files\Tesseract-OCR\tesseract.exe',
            r'C:\Program Files (x86)\Tesseract-OCR\tesseract.exe',
        ]
        for path in common_paths:
            if os.path.exists(path):
                tesseract_path = path
                print(f"Found Tesseract at: {tesseract_path}")
                break
    
    if not tesseract_path:
        print("ERROR: Tesseract not found. Please install it from: https://github.com/UB-Mannheim/tesseract/wiki")
        sys.exit(1)
    
    return tesseract_path

def find_poppler():
    """Find poppler binaries from system PATH"""
    poppler_path = None
    
    # Try 'where' command to find pdfinfo in PATH
    try:
        result = subprocess.run(['where', 'pdfinfo'], capture_output=True, text=True)
        if result.returncode == 0:
            pdfinfo_path = result.stdout.strip().split('\n')[0]
            poppler_path = os.path.dirname(pdfinfo_path)
            print(f"Found Poppler at: {poppler_path}")
    except:
        pass
    
    # If not in PATH, try common installation locations
    if not poppler_path:
        common_paths = [
            r'C:\Program Files\poppler\bin',
            r'C:\Program Files (x86)\poppler\bin',
            r'C:\poppler\bin',
        ]
        for path in common_paths:
            if os.path.exists(os.path.join(path, 'pdfinfo.exe')):
                poppler_path = path
                print(f"Found Poppler at: {poppler_path}")
                break
    
    if not poppler_path:
        print("WARNING: Poppler not found. pdf2image may not work correctly.")
        print("Download from: https://github.com/oschwartz10612/poppler-windows/releases/")
        print("Extract and add the 'bin' folder to your PATH, or press Enter to continue anyway...")
    
    return poppler_path

def ocr_pdf_to_text(pdf_path, output_txt_path=None):
    """Convert image-based PDF to selectable text"""
    
    # Check if input file exists
    if not os.path.exists(pdf_path):
        print(f"ERROR: File not found - {pdf_path}")
        return False
    
    # Set output filename if not provided
    if not output_txt_path:
        output_txt_path = pdf_path.replace('.pdf', '_extracted.txt')
    
    # Find required tools
    tesseract_cmd = find_tesseract()
    pytesseract.pytesseract.tesseract_cmd = tesseract_cmd
    
    poppler_path = find_poppler()
    
    print(f"\nProcessing: {pdf_path}")
    print("-" * 50)
    
    try:
        # Convert PDF to images
        if poppler_path:
            pages = convert_from_path(pdf_path, 300, poppler_path=poppler_path)
        else:
            pages = convert_from_path(pdf_path, 300)
        
        print(f"Total pages: {len(pages)}")
        
        full_text = ""
        for i, page in enumerate(pages):
            print(f"Processing page {i+1}/{len(pages)}...")
            text = pytesseract.image_to_string(page)
            full_text += f"\n\n--- Page {i+1} ---\n\n{text}"
        
        # Save extracted text
        with open(output_txt_path, 'w', encoding='utf-8') as f:
            f.write(full_text)
        
        print(f"\n✓ Success! Text saved to: {output_txt_path}")
        
        # Show preview
        preview = full_text.strip()[:500]
        if preview:
            print("\n--- Preview ---")
            print(preview)
            print("...")
        
        return True
        
    except Exception as e:
        print(f"\n✗ ERROR: {e}")
        print("\nTroubleshooting tips:")
        print("1. Make sure Poppler is installed and in PATH")
        print("2. Make sure Tesseract is installed and in PATH")
        print("3. Try running: pip install --upgrade pytesseract pdf2image")
        return False

if __name__ == "__main__":
    # Get PDF filename from command line or use default
    if len(sys.argv) > 1:
        pdf_file = sys.argv[1]
    else:
        pdf_file = input("Enter PDF filename: ").strip()
    
    # Optional output filename
    output_file = None
    if len(sys.argv) > 2:
        output_file = sys.argv[2]
    
    ocr_pdf_to_text(pdf_file, output_file)