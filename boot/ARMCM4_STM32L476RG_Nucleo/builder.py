import subprocess
import os

def run_cmake_build(build_dir = 'build'):
    # Remove build directory if it exists
    if os.path.exists(build_dir):
        print("Removing existing build directory")
        subprocess.run(['rm', '-rf', build_dir])

    try:
        print("Configuring project with CMake")
        subprocess.run(['cmake', '-S', '.', '-B', build_dir])

        print("Building project with CMake")
        subprocess.run(['cmake', '--build', build_dir]) 

    except subprocess.CalledProcessError as e:
        print(f"Error configuring project with CMake: {e}")
        return False
    
    print("Project built successfully")
    return True

def file_to_c_byte_array(input_file, output_file):
    # Read the input file in binary mode
    with open(input_file, 'rb') as file:
        byte_content = file.read()

    # Convert each byte to a hexadecimal string
    hex_array = [f"0x{byte:02x}" for byte in byte_content]

    # Format as a C constant byte array
    formatted_array = ', '.join(hex_array)
    c_array = f"const unsigned char array[] = {{{formatted_array}}};\n"

    # Write the output to a C file
    with open(output_file, 'w') as file:
        file.write(c_array)
    print(f"Converted {input_file} to C constant byte array in {output_file}")


run_cmake_build('build')

input_file = './build/ARMCM4_STM32L476RG_Nucleo.bin'
output_file = 'bootbin.h'
file_to_c_byte_array(input_file, output_file)