#include "sp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static	char	User[80];
static  char    Spread_name[80];
static  char    Private_group[MAX_GROUP_NAME];
static  mailbox Mbox;

static	int	Read_message();
static	void	Usage( int argc, char *argv[] );

int main( int argc, char *argv[] )
{
	int	ret;
	unsigned int	mess_len;
	char	mess[200];

	/* connecting to the daemon, requesting group information */
	ret = SP_connect( Spread_name, User, 0, 1, &Mbox, Private_group );
	if( ret < 0 ) 
	{
		SP_error( ret );
		exit(0);
	}

	/* joining a group */
	SP_join( Mbox, "simple_group" );

	printf("enter message: ");
	ret = (int) fgets( mess, 200, stdin );
	if( ret==0 ) exit(0);
	mess_len = strlen(mess);

	/* multicast a message to that group */
	ret = SP_multicast( Mbox, AGREED_MESS, "simple_group", 1, mess_len, mess );

	/* 
	 * reading a message.
	 * Note that the first message will be the membreship caused by
	 * the join (if group information was requested at connect time).
	 * If that was the case, we read another message.
	 */

	do{
		ret = Read_message();
	} while( !Is_regular_mess( ret ) );

	return 0;
}

static	int	Read_message()
{

static	char		mess[102400];
	char		sender[MAX_GROUP_NAME];
	char		target_groups[100][MAX_GROUP_NAME];
	int		num_groups;
    membership_info memb_info;
	int		service_type;
	int16		mess_type;
	int		endian_mismatch;
	int		i;
	int		ret;

	printf("\n============================\n");
        service_type = 0;
	ret = SP_receive( Mbox, &service_type, sender, 100, &num_groups, target_groups, 
		&mess_type, &endian_mismatch, sizeof(mess), mess );
	if( ret < 0 ) 
	{
		SP_error( ret );
		exit(0);
	}

	if( Is_regular_mess( service_type ) )
	{
		/* A regular message, sent by one of the processes */
		mess[ret] = 0;
		if     ( Is_unreliable_mess( service_type ) ) printf("received UNRELIABLE ");
		else if( Is_reliable_mess(   service_type ) ) printf("received RELIABLE ");
		else if( Is_fifo_mess(       service_type ) ) printf("received FIFO ");
		else if( Is_causal_mess(     service_type ) ) printf("received CAUSAL ");
		else if( Is_agreed_mess(     service_type ) ) printf("received AGREED ");
		else if( Is_safe_mess(       service_type ) ) printf("received SAFE ");
		printf("message from %s of type %d (endian %d), to %d groups \n(%d bytes): %s\n",
			sender, mess_type, endian_mismatch, num_groups, ret, mess );

	}
	else if( Is_membership_mess( service_type ) )
	{
		/* A membership notification */
		ret = SP_get_memb_info( mess, service_type, &memb_info );
		if (ret < 0) {
				printf("BUG: membership message does not have valid body\n");
				SP_error( ret );
				exit( 1 );
		}
		if ( Is_reg_memb_mess( service_type ) )
		{
			printf("received REGULAR membership ");
			if( Is_caused_join_mess( service_type ) ) printf("caused by JOIN ");
			if( Is_caused_leave_mess( service_type ) ) printf("caused by LEAVE ");
			if( Is_caused_disconnect_mess( service_type ) ) printf("caused by DISCONNECT ");
			printf("for group %s with %d members:\n",
				sender, num_groups );
			for( i=0; i < num_groups; i++ )
				printf("\t%s\n", &target_groups[i][0] );
			printf("grp id is %d %d %d\n",memb_info.gid.id[0], memb_info.gid.id[1], memb_info.gid.id[2] );
		}
		else if( Is_transition_mess(   service_type ) ) 
		{
			printf("received TRANSITIONAL membership for group %s\n", sender );
		}
		else if( Is_caused_leave_mess( service_type ) )
		{
			printf("received membership message that left group %s\n", sender );
		}else printf("received incorrect membership message of type %d\n", service_type );
	}
	else 
		printf("received message of unknown message type %d with %d bytes\n", service_type, ret);
	return( service_type );
}
