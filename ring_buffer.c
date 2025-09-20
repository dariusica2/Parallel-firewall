// SPDX-License-Identifier: BSD-3-Clause

#include "ring_buffer.h"
#include "packet.h"
#include "utils.h"

int ring_buffer_init(so_ring_buffer_t *ring, size_t cap)
{
	/* TODO: implement ring_buffer_init */

	ring->data = malloc(cap);
	DIE(ring->data == NULL, "malloc ring data failure");

	ring->read_pos = 0;
	ring->write_pos = 0;

	ring->len = 0;
	ring->cap = cap / PKT_SZ; // 1000

	pthread_mutex_init(&ring->mutex, NULL);
	pthread_cond_init(&ring->not_empty, NULL);
	pthread_cond_init(&ring->not_full, NULL);
	ring->stop = 0;

	return 1;
}

ssize_t ring_buffer_enqueue(so_ring_buffer_t *ring, void *data, size_t size)
{
	/* TODO: implement ring_buffer_enqueue */

	pthread_mutex_lock(&ring->mutex);

	while ((ring->write_pos + 1) % ring->cap == ring->read_pos)
		pthread_cond_wait(&ring->not_full, &ring->mutex);  // Wait until space is available

	memcpy(ring->data + ring->write_pos * PKT_SZ, data, size);
	ring->write_pos = (ring->write_pos + 1) % ring->cap;
	ring->len++;

	pthread_cond_signal(&ring->not_empty);

	pthread_mutex_unlock(&ring->mutex);

	return 1;
}

ssize_t ring_buffer_dequeue(so_ring_buffer_t *ring, void *data, size_t size)
{
	/* TODO: Implement ring_buffer_dequeue */

	while (ring->len == 0)
		pthread_cond_wait(&ring->not_empty, &ring->mutex);  // Wait until data is available

	memcpy(data, ring->data + ring->read_pos * PKT_SZ, size);
	ring->read_pos = (ring->read_pos + 1) % ring->cap;
	ring->len--;

	pthread_cond_signal(&ring->not_full);

	return 1;
}

void ring_buffer_destroy(so_ring_buffer_t *ring)
{
	/* TODO: Implement ring_buffer_destroy */

	pthread_cond_destroy(&ring->not_full);
	pthread_cond_destroy(&ring->not_empty);
	pthread_mutex_destroy(&ring->mutex);
	free(ring->data);
}

void ring_buffer_stop(so_ring_buffer_t *ring)
{
	/* TODO: Implement ring_buffer_stop */

	pthread_mutex_lock(&ring->mutex);

	ring->stop = 1;

	pthread_mutex_unlock(&ring->mutex);
}
