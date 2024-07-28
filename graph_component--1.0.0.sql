CREATE TYPE graph_component_hashmap;

CREATE FUNCTION graph_components_step_arr(state internal, vertex int[])
  RETURNS internal AS 'MODULE_PATHNAME', 'graph_components_step_arr'
  LANGUAGE C;

CREATE FUNCTION graph_components_final(state internal)
  RETURNS graph_component_hashmap AS 'MODULE_PATHNAME', 'graph_components_final'
  LANGUAGE C;

CREATE OR REPLACE FUNCTION get_connected_components(state graph_component_hashmap)
  RETURNS SETOF integer[] AS 'MODULE_PATHNAME', 'get_connected_components'
  LANGUAGE C;

CREATE AGGREGATE graph_components (integer[]) (
    sfunc = graph_components_step_arr,
    stype = internal,
    finalfunc = graph_components_final
);
