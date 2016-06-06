OUTPUT(lib.elf);

SECTIONS {
	text 0x0 :{
		*(.text)
	}
	.rodata :{
		*(.rodata)
	}
	.data :{
		*(.data)
		*(.sdata)
		*(.sdata2)
	}
	.bss :{
		*(.bss*)
		*(.sbss*)
		*(COMMON)
	}
	.dynsym :{
		*(.dynsym)
	}
	.dynstr :{
		*(.dynstr)
	}
	__CODE_END = .;
}