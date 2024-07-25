### MOTIVATION
It is very hard to compute graph components on pure PostgreSQL.
With this extension you can do this very efficiently. Extension does this on pointers using a minimal amount of RAM. This allows you to build components of hundreds of thousands of vertices based on many millions of pairs in seconds.

### API
**graph_components(integer[])** RETURNS internal

**get_connected_components(internal)** RETURNS SETOF integer[]

Typical usage: 
get_connected_components(graph_components(array_with_connections))
array_with_connections can contain a list of integers (id of vertex).

### EXAMPLE
Basic example:

```sql
SELECT
  get_connected_components(graph_components(array[1,2,3,4,5]))
```
Returns:
```sql
{1,2,3,4,5}
```

```sql
SELECT
  get_connected_components(graph_components(connection))
FROM (values (array[1,2,3,4,5]),
      (array[4,10]),
      (array[10,11]),
      (array[12,14]),
      (array[192]),
      (array[10000,192])
) k(connection)
```

Returns:
```sql
{12,14}
{1,2,3,4,5,10,11}
{192,10000}
```



Return components:

```sql
SELECT
  get_connected_components(graph_components(c.connections_array))
FROM public.connections c
```

Return vertex and component:

```sql
SELECT
    w.vertex,
    x.component
FROM (
  SELECT
    get_connected_components(graph_components(c.connections_array))
  FROM public.connections c
) as x(component), unnest (component) as w(vertex)
```

```sql
SELECT
    w.vertex,
    x.list
FROM (
SELECT
  get_connected_components(graph_components(connection))
FROM (values (array[1,2,3,4,5]),
      (array[4,10]),
      (array[10,11]),
      (array[12,14]),
      (array[192]),
      (array[10000,192])
) k(connection)
) as x(list), unnest (list) as w(vertex)
```
Returns:
```sql
vertex	component
12	{12,14}
14	{12,14}
1	{1,2,3,4,5,10,11}
2	{1,2,3,4,5,10,11}
3	{1,2,3,4,5,10,11}
4	{1,2,3,4,5,10,11}
5	{1,2,3,4,5,10,11}
10	{1,2,3,4,5,10,11}
11	{1,2,3,4,5,10,11}
192	{192,10000}
10000	{192,10000}
```

Example execution plan with 1.5M connections, 1078322 vertices and 239933 graph components
```sql
QUERY PLAN
Nested Loop  (cost=33952.01..34157.02 rows=10000 width=36) (actual time=16875.675..17365.712 rows=1078322 loops=1)
  ->  ProjectSet  (cost=33952.00..33957.02 rows=1000 width=32) (actual time=16875.636..17050.650 rows=239933 loops=1)
        ->  Aggregate  (cost=33952.00..33952.01 rows=1 width=32) (actual time=16875.607..16875.608 rows=1 loops=1)
              ->  Seq Scan on connections_array  (cost=0.00..30066.60 rows=1554160 width=29) (actual time=0.003..113.243 rows=1554160 loops=1)
  ->  Function Scan on unnest w  (cost=0.00..0.10 rows=10 width=4) (actual time=0.001..0.001 rows=4 loops=239933)
Planning Time: 0.428 ms
Execution Time: 17393.171 ms
```



Context:
```sql
CREATE TABLE IF NOT EXISTS public.connections_array(
    connection integer[]
);
```

