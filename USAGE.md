```

void Mesh::computeTopology() {
    int i;
    for(i = 0; i < (int)edges.size(); ++i)
        edges[i].prev = (i - i % 3) + (i + 2) % 3;
    
    vector<map<int, int> > halfEdgeMap(vertices.size());
    for(i = 0; i < (int)edges.size(); ++i) {
        int v1 = edges[i].vertex;
        int v2 = edges[edges[i].prev].vertex;

        vertices[v1].edge = edges[edges[i].prev].prev; //assign the vertex' edge
        
        if(halfEdgeMap[v1].count(v2)) {
            Debugging::out() << "Error: duplicate edge detected: " << v1 << " to " << v2 << endl;
            OUT;
        }
        halfEdgeMap[v1][v2] = i;
        if(halfEdgeMap[v2].count(v1)) {
            int twin = halfEdgeMap[v2][v1];
            edges[twin].twin = i;
            edges[i].twin = twin;
        }
        if (twin == -1 || i == -1) {
            Debugging::out() << "111" << endl;
        }
        if (i >= 99 && i <= 120) {
            Debugging::out() << i << " = " << edges[i].twin << endl;
        }
    }
}
```