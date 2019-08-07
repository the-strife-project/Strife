#!/usr/bin/env python3

'''
NASM PREPROCESSOR

Adds '%export' and '%reference' metainstructions.
'''

import sys, os

if __name__ == '__main__':
	if not len(sys.argv) == 2:
		print('Usage: %s (nasm source) [-c]' % sys.argv[0])
		exit()

	# Read the source.
	with open(sys.argv[1], 'r') as f:
		paramSource = f.read()
	source = [i.strip() for i in paramSource.split('\n')]
	source = [i for i in source if not i == '' and not i[0] == ';']

	# Check the references
	refs = [i.split('%reference ')[1] for i in source if '%reference' in i]
	refs = [i.replace('\'', '').replace('"', '') for i in refs]

	# Get the exports from each reference
	exports = {}
	for ref in refs:
		# Open the referenced file
		toOpen = os.path.dirname(sys.argv[1])
		toOpen += '/'
		toOpen += ref
		with open(toOpen, 'r') as f:
			origSource = f.read().split('\n')

		# Get the offset (will be added to address later)
		offset = '\x00'.join(origSource).split('ORG ')[1].split('\x00')[0]
		offset = int(offset, 16)

		# Parse the methods, get their names.
		source = [i.strip() for i in origSource]
		source = [i for i in source if not i == '' and not i[0] == ';']
		methods = [i.split('%export ')[1] for i in source if '%export' in i]

		for method in methods:
			# Modify the source. First, remove '%export' metainstructions.
			modSource = [i for i in origSource if not '%export' in i]

			# Now, add random bytes at the beginning of the method, so we can
			# identify it later.
			modSource = '\n'.join(modSource)
			r = os.urandom(8)
			rForSource = ', '.join([str(i) for i in r])
			modSource = modSource.replace(method+':', method+':\ndb '+rForSource+'\n')

			# Save the modified source in a temporal file.
			with open('.nasmPP.tmp', 'w') as f:
				f.write(modSource)

			# Compile.
			os.system('nasm .nasmPP.tmp -f bin -o .nasmPP.tmp.bin')

			# Check if the file exists. Otherwise, the code contains errors.
			try:
				with open('.nasmPP.tmp.bin', 'rb') as f:
					rawBytes = f.read()
			except:
				print('Nasm PP exiting...')
				exit(1)

			# Got the raw bytes. Remove the temporal files.
			os.remove('.nasmPP.tmp')
			os.remove('.nasmPP.tmp.bin')

			# Now find the random bytes.
			address = rawBytes.find(r)
			if address == -1:
				print('Wtf???? Not found! (%s)' % method)
				exit(2)
			exports[method] = hex(address+offset)

	# Last but not least, overwrite the referenced methods with their addresses.
	for i in exports:
		paramSource = paramSource.replace(i, exports[i])

	# And prepare for NASM (remove metainstructions).
	paramSource = paramSource.split('\n')
	_ = []
	for i in paramSource:
		# An empty line is left in order to mantain the line numbers in case of errors.
		_.append(i if (not '%export' in i) and (not '%reference' in i) else '')
	paramSource = _

	paramSource = '\n'.join(paramSource)

	with open(sys.argv[1]+'.npp', 'w') as f:
		f.write(paramSource)
