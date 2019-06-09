STRUCT	rfs_conf
{
	STRUCT	process_model
	{
		INT	4	process_count
	}
	
	STRUCT	node
	{
		STRING	32	id
		STRING	32	user
		STRING	32	pass
		STRUCT	server
		{
			STRING	20	ip
			INT	4	port
		}
	}
	
	STRUCT	file_system
	{
		STRING	256	root
	}
	
	STRING	5	log_level
}

