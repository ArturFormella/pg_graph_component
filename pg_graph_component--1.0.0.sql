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

