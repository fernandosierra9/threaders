#ifndef hilolay_alumnos_h__
	#define hilolay_alumnos_h__


	char *ip_suse = "127.0.0.1";
	int puerto_suse = 5003;
	int suse_fd;

	/**
	 * TODO: Interface for alumnos (what they should implement in orde to make this work)
	 */
	typedef struct hilolay_operations {
		int (*suse_create) (int);
		int (*suse_schedule_next) (void);
		int (*suse_join) (int);
		int (*suse_close) (int);
		int (*suse_wait) (int, char *);
		int (*suse_signal) (int, char *);		
	} hilolay_operations;

	hilolay_operations *main_ops;

	void init_internal(struct hilolay_operations*);

#endif // hilolay_alumnos_h__
