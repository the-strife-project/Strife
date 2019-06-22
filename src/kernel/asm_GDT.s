.global load_gdt

load_gdt:
	mov 0x4(%esp), %edx
	lgdt (%edx)
	ret
