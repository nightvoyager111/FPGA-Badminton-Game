from PIL import Image

def rgb888_to_rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

#change the image name and output file name below
def convert_png_to_c_array(image_path, array_name="tut_image", new_width=None, new_height=None, output_file="tut_image.c"):
    img = Image.open(image_path).convert('RGB')
    
    if new_width and new_height:
        img = img.resize((new_width, new_height), Image.Resampling.LANCZOS)
        print("Resized to:", img.size)


    width, height = img.size
    pixels = list(img.getdata())

    rgb565_values = [
        rgb888_to_rgb565(r, g, b) for (r, g, b) in pixels
    ]

    with open(output_file, "w") as f:
        f.write(f"#define IMAGE_WIDTH {width}\n")
        f.write(f"#define IMAGE_HEIGHT {height}\n")
        f.write(f"short int {array_name}[{width * height}] = {{\n")
        
        for i in range(height):
            row = rgb565_values[i * width:(i + 1) * width]
            row_str = ", ".join(f"0x{val:04X}" for val in row)
            f.write(f"    {row_str},\n")

        f.write("};\n")
    
    

# Place the name of the png and the size below
convert_png_to_c_array("tut.png", "tut_image", new_width=320, new_height=240)
