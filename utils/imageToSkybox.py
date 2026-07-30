import cv2
import py360convert
import os

rawPath = input("Source path: ")

if not os.path.isfile(rawPath):
    print("File not found.")
    exit(1)

e_img = cv2.imread(rawPath)

h, w, _ = e_img.shape
calculated_face_w = int(w / 4)

name_map = {
    'F': 'front',
    'B': 'back',
    'L': 'left',
    'R': 'right',
    'U': 'top',
    'D': 'bottom'
}

faces = py360convert.e2c(e_img, face_w=calculated_face_w, cube_format='dict')
for face_name, face_img in faces.items():
    full_name = name_map.get(face_name, face_name)
    cv2.imwrite(f'skybox_{full_name}.jpg', face_img)