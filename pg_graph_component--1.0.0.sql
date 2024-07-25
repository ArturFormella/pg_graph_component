/* graph_component--1.0.0.sql */

CREATE FUNCTION graph_components_step_arr(state internal, vertex int[])
  RETURNS internal AS 'MODULE_PATHNAME', 'graph_components_step_arr'
  LANGUAGE C;

CREATE FUNCTION graph_components_final(state internal)
  RETURNS bytea AS 'MODULE_PATHNAME', 'graph_components_final'
  LANGUAGE C;

CREATE OR REPLACE FUNCTION get_connected_components(state bytea)
  RETURNS SETOF integer[] AS 'MODULE_PATHNAME', 'get_connected_components'
  LANGUAGE C;

CREATE AGGREGATE graph_components (integer[]) (
    sfunc = graph_components_step_arr,
    stype = internal,
    finalfunc = graph_components_final
);



/*
--explain analyze

SELECT
  pid,value
FROM (
  SELECT
    graph_component(k.pair) as x
  FROM (SELECT * FROM "report_mkubis"."product_alternate_id" limit 100) AS k
) as xx(y), 
  unnest(y[1:1], y[2:2] ) AS pair(pid,value)


drop extension graph_component;
create extension if not exists graph_component schema public;

select
  *,
  row_number() over (partition by list)
FROM(
SELECT
 (rr->>'node_id')::int as node_id,
 sort((rr->>'list')::int[]) as list
FROM (
  SELECT
    graph_components(k.pair[1],k.pair[2]) as x

  FROM (SELECT * FROM "report_mkubis"."product_alternate_id2" where cp_id < 30000 order by cp_id desc) AS k

) as xx(y), unnest(xx.y) as u(rr)

ORDER BY 1
)
*/


