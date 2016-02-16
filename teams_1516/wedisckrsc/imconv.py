import sys
import os
import cv2
import numpy as np

if __name__ == '__main__':
	if len(sys.argv) != 3:
		print('Usage: python %s <image directory> <output file>' %
			sys.argv[0])
		sys.exit(1)

	image_dir = sys.argv[1]
	output_file = sys.argv[2]

	try:
		output_fd = open(output_file, 'w')
	except Exception as e:
		print(e)
		sys.exit(1)

	try:
		image_files = sorted(os.listdir(image_dir))
	except OSError as e:
		print(e)
		sys.exit(1)

	num_images = len(image_files)

	if num_images == 0:
		print('No images found.')
		sys.exit(1)

	print('Output file: %s' % output_file)

	for (i, image_file) in enumerate(image_files):
		sys.stdout.write('\r')
		sys.stdout.write('Processing image %u of %u' % (i, num_images - 1))
		sys.stdout.flush()

		filename = os.path.join(image_dir, image_file)
		im = cv2.imread(filename, cv2.IMREAD_UNCHANGED)
		gray_image = cv2.cvtColor(im, cv2.COLOR_BGR2GRAY)
	
		for row in range(gray_image.shape[0]):
			tmp = np.packbits(gray_image[row, :])
			output_fd.write(tmp)

	output_fd.close()
	sys.stdout.write('\n')
