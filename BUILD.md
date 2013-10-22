build nginx
===========

source code
-----------
```bash
curl http://nginx.org/download/nginx-1.4.3.tar.gz | tar xvzf -
cd nginx-1.4.3
git clone ssh://git@bitbucket.org/hotakasaito/ngx_http_json_status_module.git
```

build
-----
```bash
./configure --add-module=./ngx_http_json_status_module
make
```

debパッケージに組み込む
-----------------------
```bash
cat /etc/issue.net
Ubuntu 12.10
cat /etc/apt/sources.list.d/nginx.list
deb http://nginx.org/packages/ubuntu/ quantal nginx
deb-src http://nginx.org/packages/ubuntu/ quantal nginx
sudo apt-get update
# 依存があるので一回インストールしておく
sudo apt-get install nginx

cd /path/to/work
apt-get source nginx
rm nginx_1.4.3*
cd nginx-1.4.3
git clone ssh://git@bitbucket.org/hotakasaito/ngx_http_json_status_module.git
diff debian/{rules.orig,rules}
54c54,55
<               --with-ipv6
---
>               --with-ipv6 \
>     --add-module=./ngx_http_json_status_module
92c93,94
<               --with-debug
---
>               --with-debug \
>     --add-module=./ngx_http_json_status_module

diff debian/source/{format.orig,format}
1c1
< 3.0 (quilt)
---
> 3.0 (native)

debuild -us -uc && debclean
sudo dpkg -i ../nginx_1.4.3-1~quantalubuntu2_amd64.deb
```
