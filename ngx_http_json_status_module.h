/* -*- mode:c; coding:utf-8 -*- */

#ifndef _NGX_HTTP_JSON_STATUS_MODULE_H_INCLUDED_

#define NGX_HTTP_JSON_STATUS_MODULE_VERSION "1.0"

typedef struct ngx_http_json_status_main_conf_s ngx_http_json_status_main_conf_t;

struct  ngx_http_json_status_main_conf_s {
  char hostname[NGX_MAXHOSTNAMELEN];
};

static char *ngx_http_json_status(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_json_status_handler(ngx_http_request_t *r);

static void *ngx_http_json_status_create_main_conf(ngx_conf_t *cf);
static char *ngx_http_json_status_init_main_conf(ngx_conf_t *cf, void *conf);

#endif
