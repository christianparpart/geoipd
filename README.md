
# GeoIP Micro Service

The `geoipd` is a small program that listens on port `1337` for incoming UDP
packets that just contain a simple IPv4 address.

In turn the client will receive an UDP packet containing the country code
that IP is located in.

## The Client

The `geoipcli` tool is a small command line tool that you pass the target
server to send your query to and as second argument the IPv4 address you
are to lookup the country code for. This country code will be printed to
`stdout`.
