#ifndef RSPAMD_MAP_H
#define RSPAMD_MAP_H

#include "config.h"
#include "mem_pool.h"
#include "radix.h"

/**
 * Maps API is designed to load lists data from different dynamic sources.
 * It monitor files and HTTP locations for modifications and reload them if they are
 * modified.
 */

enum fetch_proto {
	MAP_PROTO_FILE,
	MAP_PROTO_HTTP,
};
struct map_cb_data;

/**
 * Callback types
 */
typedef gchar * (*map_cb_t)(rspamd_mempool_t *pool, gchar *chunk, gint len,
	struct map_cb_data *data);
typedef void (*map_fin_cb_t)(rspamd_mempool_t *pool, struct map_cb_data *data);

/**
 * Common map object
 */
struct rspamd_config;
struct rspamd_map {
	rspamd_mempool_t *pool;
	struct rspamd_config *cfg;
	enum fetch_proto protocol;
	map_cb_t read_callback;
	map_fin_cb_t fin_callback;
	void **user_data;
	struct event ev;
	struct timeval tv;
	struct event_base *ev_base;
	void *map_data;
	gchar *uri;
	gchar *description;
	guint32 id;
	guint32 checksum;
	/* Shared lock for temporary disabling of map reading (e.g. when this map is written by UI) */
	gint *locked;
};

/**
 * Callback data for async load
 */
struct map_cb_data {
	struct rspamd_map *map;
	gint state;
	void *prev_data;
	void *cur_data;
};


/**
 * Check map protocol
 */
gboolean check_map_proto (const gchar *map_line, gint *res, const gchar **pos);
/**
 * Add map from line
 */
gboolean add_map (struct rspamd_config *cfg,
	const gchar *map_line,
	const gchar *description,
	map_cb_t read_callback,
	map_fin_cb_t fin_callback,
	void **user_data);

/**
 * Start watching of maps by adding events to libevent event loop
 */
void start_map_watch (struct rspamd_config *cfg, struct event_base *ev_base);

/**
 * Remove all maps watched (remove events)
 */
void remove_all_maps (struct rspamd_config *cfg);

typedef void (*insert_func) (gpointer st, gconstpointer key,
	gconstpointer value);

/**
 * Common callbacks for frequent types of lists
 */

/**
 * Radix list is a list like ip/mask
 */
gchar * read_radix_list (rspamd_mempool_t *pool,
	gchar *chunk,
	gint len,
	struct map_cb_data *data);
void fin_radix_list (rspamd_mempool_t *pool, struct map_cb_data *data);

/**
 * Host list is an ordinal list of hosts or domains
 */
gchar * read_host_list (rspamd_mempool_t *pool,
	gchar *chunk,
	gint len,
	struct map_cb_data *data);
void fin_host_list (rspamd_mempool_t *pool, struct map_cb_data *data);

/**
 * Kv list is an ordinal list of keys and values separated by whitespace
 */
gchar * read_kv_list (rspamd_mempool_t *pool,
	gchar *chunk,
	gint len,
	struct map_cb_data *data);
void fin_kv_list (rspamd_mempool_t *pool, struct map_cb_data *data);

/**
 * FSM for lists parsing (support comments, blank lines and partial replies)
 */
gchar * abstract_parse_list (rspamd_mempool_t * pool,
	gchar * chunk,
	gint len,
	struct map_cb_data *data,
	insert_func func);

#endif
