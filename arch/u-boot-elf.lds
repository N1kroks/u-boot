ENTRY(CONFIG_PLATFORM_ELFENTRY)
PHDRS
{
	data PT_LOAD FLAGS(7);
}
SECTIONS
{
	. = CONFIG_PLATFORM_ELFENTRY;

	.data : {
		*(.data*)
	}: data
}
