/* -*- mode:c; coding:utf-8 -*- */

#include <nginx.h>
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

  ngx_http_json_status_create_main_conf, /* create main configuration */
  ngx_http_json_status_init_main_conf,   /* init main configuration */

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

static void *
ngx_http_json_status_create_main_conf(ngx_conf_t *cf)
{
  ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "%s #start.", __FUNCTION__);

  ngx_http_json_status_main_conf_t *mcf;

  mcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_json_status_main_conf_t));
  if (mcf == NULL) {
    return NULL;
  }

  if (gethostname(mcf->hostname, NGX_MAXHOSTNAMELEN) == -1) {
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "gethostname() failed");
    return NULL;
  }  

  ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "hostname:%s", &mcf->hostname);

  return mcf;
}

static char *
ngx_http_json_status_init_main_conf(ngx_conf_t *cf, void *conf)
{
  ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "#start. %s:%d", __FUNCTION__, __LINE__);
  return NGX_CONF_OK;
}


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

  size = sizeof("{}")+
    sizeof("\"version\": \"\",")+sizeof(NGX_HTTP_JSON_STATUS_MODULE_VERSION)+
    sizeof("\"nginx_version\": \"\",")+sizeof(NGINX_VERSION)+
    sizeof("\"address\": \"\",")+sizeof(u_char)+
    sizeof("\"timestamp\": \"\",")+sizeof(u_char)
    ;

  b = ngx_create_temp_buf(r->pool, size);
  if (b == NULL) {
    return NGX_HTTP_INTERNAL_SERVER_ERROR;
  }

  out.buf = b;
  out.next = NULL;

  b->last = ngx_sprintf(b->last, "{"); /* contents start */
  b->last = ngx_sprintf(b->last, "\"version\": \"%s\",", NGX_HTTP_JSON_STATUS_MODULE_VERSION); /* module version */
  b->last = ngx_sprintf(b->last, "\"nginx_version\": \"%s\",", NGINX_VERSION); /* nginx version */
  b->last = ngx_sprintf(b->last, "}"); /* contents end */

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
  ngx_http_json_status_main_conf_t *mcf = conf;

  clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
  clcf->handler = ngx_http_json_status_handler;

  mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_json_status_module);
  ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "#hostname: %s. %s:%d", &mcf->hostname, __FUNCTION__, __LINE__);

  return NGX_CONF_OK;
}
