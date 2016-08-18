/*
 * Sample demo application that showcases inter processor
 * communication from linux userspace to a remote software
 * context. The application generates random matrices and
 * transmits them to the remote context over rpmsg. The
 * remote application performs multiplication of matrices
 * and transmits the results back to this application.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>

#define MATRIX_SIZE 6

/* Shutdown message ID */
#define SHUTDOWN_MSG	0xEF56A55A

struct _matrix {
	unsigned int size;
	unsigned int elements[MATRIX_SIZE][MATRIX_SIZE];
};

static void matrix_print(struct _matrix *m)
{
	int i, j;

	/* Generate two random matrices */
	printf(" \r\n Master : Linux : Printing results \r\n");

	for (i = 0; i < m->size; ++i) {
		for (j = 0; j < m->size; ++j)
			printf(" %d ", (unsigned int)m->elements[i][j]);
		printf("\r\n");
	}
}

static void generate_matrices(int num_matrices,
			      unsigned int matrix_size, void *p_data)
{
	int i, j, k;
	struct _matrix *p_matrix = p_data;
	time_t t;
	unsigned long value;

	srand((unsigned)time(&t));

	for (i = 0; i < num_matrices; i++) {
		/* Initialize workload */
		p_matrix[i].size = matrix_size;

		printf(" \r\n Master : Linux : Input matrix %d \r\n", i);
		for (j = 0; j < matrix_size; j++) {
			printf("\r\n");
			for (k = 0; k < matrix_size; k++) {

				value = (rand() & 0x7F);
				value = value % 10;
				p_matrix[i].elements[j][k] = value;
				printf(" %d ",
				       (unsigned int)p_matrix[i].
				       elements[j][k]);
			}
		}
		printf("\r\n");
	}

}

static pthread_t ui_thread, compute_thread;
static pthread_mutex_t sync_lock;

static int fd, compute_flag;
static struct _matrix i_matrix[2];
static struct _matrix r_matrix;

#define RPMSG_GET_KFIFO_SIZE 1
#define RPMSG_GET_FREE_SPACE 3

void *ui_thread_entry(void *ptr)
{
	int cmd, ret;
	int flag = 1;

	while (flag) {
		printf("\r\n **************************************** \r\n");
		printf(" Please enter command and press enter key\r\n");
		printf(" **************************************** \r\n");
		printf(" 1 - Generates random 6x6 matrices and transmits");
		printf(" them to remote core over rpmsg .. \r\n");
		printf(" 2 - Quit this application .. \r\n");
		printf(" CMD>");
		ret = scanf("%d", &cmd);
		if (!ret) {
			while (1) {
				if (getchar() == '\n')
					break;
			}

			printf("\r\n invalid command\r\n");
			continue;
		}
		if (cmd == 1) {
			compute_flag = 1;
			pthread_mutex_unlock(&sync_lock);

			printf("\r\n Compute thread unblocked .. \r\n");
			printf(" The compute thread is now blocking on");
			printf("a read() from rpmsg device \r\n");
			printf("\r\n Generating random matrices now ... \r\n");

			generate_matrices(2, 6, i_matrix);

			printf("\r\n Writing generated matrices to rpmsg ");
			printf("rpmsg device, %d bytes written .. \r\n",
			       sizeof(i_matrix));

			write(fd, i_matrix, sizeof(i_matrix));

			/* adding this so the threads
			   dont overlay the strings they print */
			sleep(1);
		} else if (cmd == 2) {
			flag = 0;
			compute_flag = 0;
			pthread_mutex_unlock(&sync_lock);
			printf("\r\n Quitting application .. \r\n");
			printf(" Matrix multiplication demo end \r\n");
		} else {
			printf("\r\n invalid command! \r\n");
		}
	}

	return 0;
}

void *compute_thread_entry(void *ptr)
{
	int bytes_rcvd;

	pthread_mutex_lock(&sync_lock);

	while (compute_flag == 1) {

		do {
			bytes_rcvd = read(fd, &r_matrix, sizeof(r_matrix));
		} while ((bytes_rcvd < sizeof(r_matrix)) || (bytes_rcvd < 0));

		printf("\r\n Received results! - %d bytes from ", bytes_rcvd);
		printf("rpmsg device (transmitted from remote context) \r\n");

		matrix_print(&r_matrix);

		pthread_mutex_lock(&sync_lock);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	unsigned int size;
	int shutdown_msg = SHUTDOWN_MSG;

	printf("\r\n Matrix multiplication demo start \r\n");

	printf("\r\n Open rpmsg dev! \r\n");

	fd = open("/dev/rpmsg0", O_RDWR);

	printf("\r\n Query internal info .. \r\n");

	ioctl(fd, RPMSG_GET_KFIFO_SIZE, &size);

	printf(" rpmsg kernel fifo size = %u \r\n", size);

	ioctl(fd, RPMSG_GET_FREE_SPACE, &size);

	printf(" rpmsg kernel fifo free space = %u \r\n", size);

	if (pthread_mutex_init(&sync_lock, NULL) != 0)
		printf("\r\n mutex initialization failure \r\n");

	pthread_mutex_lock(&sync_lock);

	printf("\r\n Creating ui_thread and compute_thread ... \r\n");

	pthread_create(&ui_thread, NULL, &ui_thread_entry, "ui_thread");

	pthread_create(&compute_thread, NULL, &compute_thread_entry,
		       "compute_thread");
	pthread_join(ui_thread, NULL);

	pthread_join(compute_thread, NULL);

	/* Send shutdown message to remote */
	write(fd, &shutdown_msg, sizeof(int));
	sleep(1);

	printf("\r\n Quitting application .. \r\n");
	printf(" Matrix multiply application end \r\n");

	close(fd);

	pthread_mutex_destroy(&sync_lock);

	return 0;
}
