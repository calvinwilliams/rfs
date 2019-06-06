STRUCT	rfs_conf
{
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
	
	STRING	256	root
}

