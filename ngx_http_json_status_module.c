/* -*- mode:c; coding:utf-8 -*- */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_http_json_status_module.h"

static ngx_command_t
ngx_http_json_status_commands[] = {
  {
    ngx_string("status"),
    NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
    ngx_http_json_status,
    0,
    0,
    NULL
  },
  ngx_null_command
};

static ngx_http_module_t ngx_http_json_status_module_ctx = {
  NULL,                              /* preconfiguration */
  NULL,                              /* postconfiguration */

  NULL,                              /* create main configuration */
  NULL,                              /* init main configuration */

  NULL,                              /* create server configuration */
  NULL,                              /* merge server configuration */

  NULL,                              /* create location configuration */
  NULL                               /* merge location configuration */
};

ngx_module_t ngx_http_json_status_module = {
  NGX_MODULE_V1,
  &ngx_http_json_status_module_ctx, /* module context */
  ngx_http_json_status_commands,    /* module directives */
  NGX_HTTP_MODULE,                      /* module type */
  NULL,                                 /* init master */
  NULL,                                 /* init module */
  NULL,                                 /* init process */
  NULL,                                 /* init thread */
  NULL,                                 /* exit thread */
  NULL,                                 /* exit process */
  NULL,                                 /* exit master */
  NGX_MODULE_V1_PADDING
};

static ngx_int_t
ngx_http_json_status_handler(ngx_http_request_t *r)
{
  size_t       size;
  ngx_buf_t   *b;
  ngx_int_t    rc;
  ngx_chain_t  out;

  ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "#start. %s:%d", __FUNCTION__, __LINE__);

  /* GET or HEAD only */
  if (r->method != NGX_HTTP_GET && r->method != NGX_HTTP_HEAD) {
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "method = %d: GET = %d,HEAD = %d", r->method, NGX_HTTP_GET, NGX_HTTP_HEAD);
    return NGX_HTTP_NOT_ALLOWED;
  }

  /* request bodyは不要なので破棄する */
  rc = ngx_http_discard_request_body(r);
  if (rc != NGX_OK) {
    return rc;
  }

  size = sizeof("{}");

  b = ngx_create_temp_buf(r->pool, size);
  if (b == NULL) {
    return NGX_HTTP_INTERNAL_SERVER_ERROR;
  }

  out.buf = b;
  out.next = NULL;

  b->last = ngx_sprintf(b->last, "{}");

  b->memory = 1;
  b->flush = 1;
  b->last_buf = 1;
  b->last_in_chain = 1;

  ngx_str_set(&r->headers_out.content_type, "application/json; charset=utf-8");
  r->headers_out.status = NGX_HTTP_OK;
  r->headers_out.content_length_n = b->last - b->pos;
  rc = ngx_http_send_header(r);
  if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "#return. %s:%d", __FUNCTION__, __LINE__);
    return rc;
  }

  return ngx_http_output_filter(r, &out);
}

static char *
ngx_http_json_status(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
  ngx_http_core_loc_conf_t  *clcf;

  clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
  clcf->handler = ngx_http_json_status_handler;

  return NGX_CONF_OK;
}
