#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "csvParse.h"
#include "mode.h"
#include "nodesAndEdges.h"

/* ------------------ Utility ------------------ */

void trim_in_place(char *s) {
    if (!s) return;

    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;

    if (p != s)
        memmove(s, p, strlen(p) + 1);

    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1]))
        s[--n] = '\0';
}

/* ------------------ CSV Splitter ------------------ */
/* Pure standard C — works on Windows, Linux, Mac */

int split_csv(char *line, char **tokens, int maxTokens) {
    int count = 0;

    char *tok = strtok(line, ",");
    while (tok && count < maxTokens) {
        trim_in_place(tok);
        tokens[count++] = tok;
        tok = strtok(NULL, ",");
    }

    return count;
}

int is_number_token(const char *s) {
    if (!s) return 0;

    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return 0;

    char *end = NULL;
    strtod(s, &end);

    while (end && *end && isspace((unsigned char)*end)) end++;
    return end && *end == '\0';
}

/* ------------------ Road CSV ------------------ */

void parseRoadmapCSV(const char *filename) {

    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Error opening %s\n", filename);
        return;
    }

    char line[MAX_LINE];
    char *tokens[MAX_TOKENS];

    while (fgets(line, sizeof(line), f)) {

        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        int count = split_csv(line, tokens, MAX_TOKENS);
        if (count < 6) continue;

        const char *altTok = tokens[count - 2];
        const char *lenTok = tokens[count - 1];

        if (!is_number_token(altTok) || !is_number_token(lenTok))
            continue;

        int coordCount = count - 3;
        if (coordCount < 4 || coordCount % 2 != 0)
            continue;

        for (int i = 1; i + 3 <= count - 2; i += 2) {

            double lon1 = atof(tokens[i]);
            double lat1 = atof(tokens[i + 1]);
            double lon2 = atof(tokens[i + 2]);
            double lat2 = atof(tokens[i + 3]);

            int from = findOrAddNode(lat1, lon1);
            int to   = findOrAddNode(lat2, lon2);

            double dist = haversineDistance(lat1, lon1, lat2, lon2);

            addEdge(from, to, MODE_CAR, dist);
            addEdge(to, from, MODE_CAR, dist);
        }
    }

    fclose(f);
}

/* ------------------ Metro CSV ------------------ */

void parseMetroCSV(const char *filename) {

    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Error opening %s\n", filename);
        return;
    }

    char line[MAX_LINE];
    char *tokens[MAX_TOKENS];

    while (fgets(line, sizeof(line), f)) {

        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        int count = split_csv(line, tokens, MAX_TOKENS);
        if (count < 5) continue;

        const char *startStation = tokens[count - 2];
        const char *endStation   = tokens[count - 1];

        if (is_number_token(startStation) || is_number_token(endStation))
            continue;

        int coordCount = count - 3;
        if (coordCount < 4 || coordCount % 2 != 0)
            continue;

        for (int i = 1; i + 3 <= count - 2; i += 2) {

            double lon1 = atof(tokens[i]);
            double lat1 = atof(tokens[i + 1]);
            double lon2 = atof(tokens[i + 2]);
            double lat2 = atof(tokens[i + 3]);

            int from = findOrAddNode(lat1, lon1);
            int to   = findOrAddNode(lat2, lon2);

            double dist = haversineDistance(lat1, lon1, lat2, lon2);

            addEdge(from, to, MODE_METRO, dist);
            addEdge(to, from, MODE_METRO, dist);

            if (i == 1) {
                strncpy(nodes[from].name, startStation,
                        sizeof(nodes[from].name) - 1);
                nodes[from].name[sizeof(nodes[from].name) - 1] = '\0';
            }

            if (i + 4 > count - 2) {
                strncpy(nodes[to].name, endStation,
                        sizeof(nodes[to].name) - 1);
                nodes[to].name[sizeof(nodes[to].name) - 1] = '\0';
            }
        }
    }

    fclose(f);
}

/* ------------------ Bus CSV ------------------ */

void parseBusCSV(const char *filename, Mode busMode) {

    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Error opening %s\n", filename);
        return;
    }

    char line[MAX_LINE];
    char *tokens[MAX_TOKENS];

    while (fgets(line, sizeof(line), f)) {

        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        int count = split_csv(line, tokens, MAX_TOKENS);
        if (count < 5) continue;

        const char *startStation = tokens[count - 2];
        const char *endStation   = tokens[count - 1];

        if (is_number_token(startStation) || is_number_token(endStation))
            continue;

        int coordCount = count - 3;
        if (coordCount < 4 || coordCount % 2 != 0)
            continue;

        for (int i = 1; i + 3 <= count - 2; i += 2) {

            double lon1 = atof(tokens[i]);
            double lat1 = atof(tokens[i + 1]);
            double lon2 = atof(tokens[i + 2]);
            double lat2 = atof(tokens[i + 3]);

            int from = findOrAddNode(lat1, lon1);
            int to   = findOrAddNode(lat2, lon2);

            double dist = haversineDistance(lat1, lon1, lat2, lon2);

            addEdge(from, to, busMode, dist);
            addEdge(to, from, busMode, dist);

            if (i == 1) {
                strncpy(nodes[from].name, startStation,
                        sizeof(nodes[from].name) - 1);
                nodes[from].name[sizeof(nodes[from].name) - 1] = '\0';
            }

            if (i + 4 > count - 2) {
                strncpy(nodes[to].name, endStation,
                        sizeof(nodes[to].name) - 1);
                nodes[to].name[sizeof(nodes[to].name) - 1] = '\0';
            }
        }
    }

    fclose(f);
}

/* ------------------ KML Export ------------------ */

void exportPathToKML(int path[], int pathLen, const char *filename) {

    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("Failed to open %s\n", filename);
        return;
    }

    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<kml xmlns=\"http://earth.google.com/kml/2.1\">\n");
    fprintf(f, "<Document>\n<Placemark>\n<name>Route</name>\n");
    fprintf(f, "<LineString>\n<tessellate>1</tessellate>\n<coordinates>\n");

    for (int i = pathLen - 1; i >= 0; i--) {
        int id = path[i];
        fprintf(f, "%.6f,%.6f,0\n", nodes[id].lon, nodes[id].lat);
    }

    fprintf(f, "</coordinates>\n</LineString>\n</Placemark>\n</Document>\n</kml>\n");

    fclose(f);
    printf("Exported path to %s\n", filename);
}
