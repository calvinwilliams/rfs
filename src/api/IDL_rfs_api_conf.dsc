STRUCT	rfs_api_conf
{
	STRUCT	nodes	ARRAY	16
	{
		STRING	32	id
		STRING	32	user
		STRING	32	pass
		STRUCT	servers	ARRAY	8
		{
			STRING	20	ip
			INT	4	port
		}
	}
}

