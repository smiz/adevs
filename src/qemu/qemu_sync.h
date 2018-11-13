#ifndef _qemu_sync_h_
#define _qemu_sync_h_
#include <semaphore.h>

/**
 * Simulator side protocol for coordinating time
 * advanced with QEMU.
 */
class qemu_sync
{
	public:
		/**
		 * Start initializing the synchronization protocol.
		 * The handshake() method will block until
		 * initialization is finished. Only one
		 * thread at a time may setup a synchronization
		 * channel. The next thread to call this
		 * constructor will block until the first
		 * thread calls handshake.
		 */
		qemu_sync();
		/**
		 * Call after creating the qemu_sync object
		 * and before any calls to run and sync.
		 * This will block until the synchronization
		 * protocol is initialized.
		 */
		void handshake();
		/**
		 * Get the actual nanoseconds that QEMU has advanced
		 * its simulation clock and the time h_ns until its
		 * next event. Call this first. If QEMU is running,
		 * this method will block until it reaches the time
		 * advance allowed by the prior call to run(). Otherwise
		 * it will return e_ns = 0 and h_ns = to the time of
		 * the first event.
		 */
		void sync(long* e_ns, long* h_ns);
		/**
		 * Tell QEMU to execute for h nanoseconds of
		 * simulation time. Returns immediately and
		 * QEMU executes asynchronously. The value of h_ns
		 * must be less than or equal to the time advance
		 * required by sync().
		 */
		void run(long h_ns);
		/**
		 * Tear down the synchronization channel.
		 */
		~qemu_sync();
	private:
		sem_t* sem[3];
		long* buf;
		int mem_fd;
};

#endif
