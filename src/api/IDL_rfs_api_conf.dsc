STRUCT	rfs_api_conf
{
	STRUCT	log
	{
		STRING	5	log_level
	}
	STRUCT	servers	ARRAY	8
	{
		STRING	20	ip
		INT	4	port
	}
	STRUCT	comm
	{
		INT	4	connecting_timeout
	}
}

