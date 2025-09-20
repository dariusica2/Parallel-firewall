// SPDX-License-Identifier: BSD-3-Clause

#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

#include "consumer.h"
#include "ring_buffer.h"
#include "packet.h"
#include "utils.h"

int out_fd;
pthread_mutex_t write_mutex;
void *ctx;

void *consumer_thread(void *ctx)
{
	/* TODO: implement consumer thread */
	char packet_data[PKT_SZ], out_buf[PKT_SZ];
	int len;

	so_ring_buffer_t *ring = ((so_consumer_ctx_t *)ctx)->producer_rb;

	while (1) {
		pthread_mutex_lock(&ring->mutex);

		if (ring->len == 0 && ring->stop) {
			pthread_mutex_unlock(&ring->mutex);
			break;
		}

		int dq = ring_buffer_dequeue(((so_consumer_ctx_t *)ctx)->producer_rb, packet_data, PKT_SZ);

		pthread_mutex_unlock(&ring->mutex);

		if (dq) {
			struct so_packet_t *pkt = (struct so_packet_t *)packet_data;

			int action = process_packet(pkt);
			unsigned long hash = packet_hash(pkt);
			unsigned long timestamp = pkt->hdr.timestamp;

			pthread_mutex_lock(&write_mutex);

			len = snprintf(out_buf, 256, "%s %016lx %lu\n", RES_TO_STR(action), hash, timestamp);
			write(out_fd, out_buf, len);

			pthread_mutex_unlock(&write_mutex);
		}
	}

	return NULL;
}

int create_consumers(pthread_t *tids,
					 int num_consumers,
					 struct so_ring_buffer_t *rb,
					 const char *out_filename)
{
	out_fd = open(out_filename, O_RDWR|O_CREAT|O_TRUNC, 0666);
	DIE(out_fd < 0, "opening output file failure");

	pthread_mutex_init(&write_mutex, NULL);

	ctx = malloc(sizeof(so_consumer_ctx_t));
	DIE(!ctx, "malloc so_consumer_ctx_t error");

	((so_consumer_ctx_t *)ctx)->producer_rb = rb;
	((so_consumer_ctx_t *)ctx)->out_filename = out_filename;

	for (int i = 0; i < num_consumers; i++) {
		/* TODO: Launch consumer threads */
		int r = pthread_create(&tids[i], NULL, consumer_thread, ctx);

		DIE(r, "pthread_create error");
	}

	return num_consumers;
}

void end_consumer_threads(void)
{
	free(ctx);
	pthread_mutex_destroy(&write_mutex);
	close(out_fd);
}
