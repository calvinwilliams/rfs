STRUCT	rfs_conf
{
	STRUCT	process_model
	{
		INT	4	process_count
	}
	
	STRUCT	listen
	{
		STRING	20	ip
		INT	4	port
	}
	
	STRUCT	fs
	{
		STRING	256	root
	}
	
	STRUCT	log
	{
		STRING	5	log_level
	}
}

