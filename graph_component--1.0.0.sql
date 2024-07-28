CREATE TYPE graph_component_hashmap;

CREATE FUNCTION graph_components_step_arr(state internal, vertex int[])
  RETURNS internal AS 'MODULE_PATHNAME', 'graph_components_step_arr'
  LANGUAGE C;

CREATE FUNCTION graph_components_final(state internal)
  RETURNS graph_component_hashmap AS 'MODULE_PATHNAME', 'graph_components_final'
  LANGUAGE C;

CREATE OR REPLACE FUNCTION get_component(state graph_component_hashmap)
  RETURNS SETOF int4[] AS 'MODULE_PATHNAME', 'get_component'
  LANGUAGE C;

CREATE OR REPLACE FUNCTION get_component_id(state graph_component_hashmap)
  RETURNS TABLE(id int4, component_id int4) AS 'MODULE_PATHNAME', 'get_component_id'
  LANGUAGE C;

CREATE AGGREGATE graph_components (int4[]) (
    sfunc = graph_components_step_arr,
    stype = internal,
    finalfunc = graph_components_final
);
