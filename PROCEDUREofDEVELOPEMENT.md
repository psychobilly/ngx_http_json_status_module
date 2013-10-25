モジュール開発
==============

雛形作成
--------
```bash
sudo gem install ngxmodgen
mkdir ngx_http_json_status_module
cd ngx_http_json_status_module
ngxmodgen -n json_status
tree
.
├── config
└── ngx_http_json_status_module.c

0 directories, 2 files
```

ディレクティブの設定
--------------------
[Configuration directives](http://www.nginxguts.com/2011/09/configuration-directives/)
### status;
```c
static char *ngx_http_json_status(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
...
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
```
### status on|off; の場合
```c
static char *ngx_http_json_status(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
...
static ngx_command_t
ngx_http_json_status_commands[] = {
  {
    ngx_string("status"),
    NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
    ngx_http_json_status,
    0,
    0,
    NULL
  },
  ngx_null_command
};
```

ヘッダファイルを追加
--------------------
ヘッダファイルを作りたくなった場合configに一行追加する。  
今回はngx_http_json_status_module.hを追加するのでconfigに以下を追加。
```
NGX_ADDON_DEPS="$NGX_ADDON_DEPS $ngx_addon_dir/ngx_http_json_status_module.h"
```

ngx_command_t
-------------
ディレクティブの定義をした ngx_command_t 構造体は以下の様になっている。
```c
typedef struct ngx_command_s     ngx_command_t;

struct ngx_command_s {
    ngx_str_t             name;
    ngx_uint_t            type;
    char               *(*set)(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
    ngx_uint_t            conf;
    ngx_uint_t            offset;
    void                 *post;
};
```
set要素はname要素のディレクティブが設定されたときに呼び出される関数。

ngx_command_tのset要素:ngx_http_json_status
-------------------------------------------
status; が設定されているときに呼び出す関数。  
内部でhandlerを設定する。
### ngx_http_json_status_module.h
```c
static char *ngx_http_json_status(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
```
### ngx_http_json_status_module.c
```c
static char *
ngx_http_json_status(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
  ngx_http_core_loc_conf_t  *clcf;

  clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
  clcf->handler = ngx_http_json_status_handler;

  return NGX_CONF_OK;
}
```

handler:ngx_http_json_status_handler
------------------------------------
status;が設定されたlocationにリクエストが来た時に実行される。
### ngx_http_json_status_module.h
```c
static ngx_int_t ngx_http_json_status_handler(ngx_http_request_t *r);
```
### ngx_http_json_status_module.c
jsonで空データを返すところまで  
```c
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
```

create main configuration
------------
モジュール内のメインの設定用の構造体を生成する。
### ngx_http_json_status_module.h
```c
struct  ngx_http_json_status_main_conf_s {
  char            hostname[NGX_MAXHOSTNAMELEN];
  u_char          addr[16]; /* xxx.xxx.xxx.xxx\0 */
};
...
static void *ngx_http_json_status_create_main_conf(ngx_conf_t *cf);
```
### ngx_http_json_status_module.c
* ngx_http_json_status_module_ctxのcreate_main_confに構造体を生成する関数を指定
* ngx_http_json_status_create_main_confでhostnameとip addressを取得してセットする
```c
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
...
static void *
ngx_http_json_status_create_main_conf(ngx_conf_t *cf)
{
  ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "%s #start.", __FUNCTION__);

  ngx_http_json_status_main_conf_t *jsmcf;
  struct hostent *host;

  jsmcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_json_status_main_conf_t));
  if (jsmcf == NULL) {
    return NULL;
  }

  if (gethostname(jsmcf->hostname, NGX_MAXHOSTNAMELEN) == -1) {
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "gethostname() failed");
    return NULL;
  }
  host = gethostbyname(jsmcf->hostname);
  if (host == NULL) {
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "gethostbyname() failed");
    return NULL;
  }
  ngx_sprintf(jsmcf->addr, "%d.%d.%d.%d",
	      (BYTE)*(host->h_addr),
	      (BYTE)*(host->h_addr + 1),
	      (BYTE)*(host->h_addr + 2),
	      (BYTE)*(host->h_addr + 3));

  ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "hostname:%s, address:%s", &jsmcf->hostname, jsmcf->addr);

  return jsmcf;
}
```
