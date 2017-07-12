#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_str_t hello_string;
    ngx_int_t hello_counter;
} ngx_http_hello_loc_conf_t;

static ngx_int_t ngx_http_hello_init(ngx_conf_t *cf);

static void *ngx_http_hello_create_loc_conf(ngx_conf_t *cf);

static char *ngx_http_hello_string(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_hello_counter(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t ngx_http_hello_commands[] = {
    {
        ngx_string("hello_string"),
        NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS | NGX_CONF_TAKE1,
        ngx_http_hello_string,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_hello_loc_conf_t, hello_string),
        NULL
    },
    {
        ngx_string("hello_counter"),
        NGX_HTTP_LOC_CONF | NGX_CONF_FLAG,
        ngx_http_hello_counter,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_hello_loc_conf_t, hello_counter),
        NULL
    },
    ngx_null_command
};

static int ngx_hello_visited_times = 0;

static ngx_http_module_t ngx_http_hello_module_ctx = {
    NULL,
    ngx_http_hello_init,
    NULL,
    NULL,
    NULL,
    NULL,
    ngx_http_hello_create_loc_conf,
    NULL
};

ngx_module_t ngx_http_hello_module = {
    NGX_MODULE_V1,
    &ngx_http_hello_module_ctx,
    ngx_http_hello_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING
};


static ngx_int_t ngx_http_hello_handler(ngx_http_request_t *r)
{
   // ngx_int_t rc;
    ngx_buf_t *b;
    ngx_chain_t out;
    ngx_http_hello_loc_conf_t * my_conf;
    u_char ngx_hello_string[1024] = {0};
    ngx_uint_t content_length = 0;

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_hello_handler is called!");
    
    my_conf = ngx_http_get_module_loc_conf(r, ngx_http_hello_module);

    ngx_sprintf(ngx_hello_string, "%s Visited Times:%d", my_conf->hello_string.data, ++ngx_hello_visited_times);

    content_length = ngx_strlen(ngx_hello_string);

    ngx_http_discard_request_body(r);

    ngx_str_set(&r->headers_out.content_type, "text/html");

    if(r->method == NGX_HTTP_HEAD)
    {
        r->headers_out.status = NGX_HTTP_OK;
        r->headers_out.content_length_n = content_length;
        
        return ngx_http_send_header(r);

    }

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    out.buf = b;
    out.next = NULL;

    b->pos = ngx_hello_string;
    b->last = ngx_hello_string + content_length;
    b->memory = 1;
    b->last_buf = 1;

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = content_length;

    ngx_http_send_header(r);

    return ngx_http_output_filter(r, &out);
}

/*
static u_char ngx_hello_default_string[] = "Default String: hello world!";

static char *ngx_http_hello_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_hello_loc_conf_t *prev = parent;
    ngx_http_hello_loc_conf_t *conf = child;

    ngx_conf_merge_str_value(conf->hello_string, prev->hello_string, ngx_hello_default_string);
    ngx_conf_merge_value(conf->hello_counter, prev->hello_counter, 0);

    return NGX_CONF_OK;
}
*/

static void *ngx_http_hello_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_hello_loc_conf_t *local_conf = NULL;
    local_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_hello_loc_conf_t));
    ngx_str_null(&local_conf->hello_string);
    local_conf->hello_counter = -1;

    return local_conf;
}

static char *ngx_http_hello_string(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_hello_loc_conf_t *local_conf;

    local_conf = conf;

    char *rv = ngx_conf_set_str_slot(cf, cmd, conf);

    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "hello_string:%s", local_conf->hello_string.data);

    return rv;
}


static char *ngx_http_hello_counter(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_hello_loc_conf_t *local_conf;

    local_conf = conf;

    char *rv = NULL;

    rv = ngx_conf_set_flag_slot(cf, cmd, conf);

    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "hello_counter:%d", local_conf->hello_counter);

    return rv;
}


static ngx_int_t ngx_http_hello_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *cmcf;
    
    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL)
    {
        return NGX_ERROR;
    }
    
    *h = ngx_http_hello_handler;
    
    return NGX_OK;
}
    
