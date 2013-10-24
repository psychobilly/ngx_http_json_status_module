nginx tips
==========

ngx_conf_log_error
------------------
NGX_LOG_DEBUGを使用する場合 --with-debug オプションを付けてbuildする必要あり。  
```c
ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "test");
```
