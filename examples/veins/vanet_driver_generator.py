import xml.etree.ElementTree as ET
from random import choice


class DriverGenerator:
    vQuantity = 0
    delays = {}
    in_edges = []
    out_edges = []

    def __init__(self, filename='additional_config.xml'):
        config = ET.parse(filename)
        root = config.getroot()
        self.vQuantity = int(root.find('vehicles').find('quantity').attrib['value'])
        for pair in root.find('vehicles').find('reflex_dist').findall('delay'):
            self.delays.update({float(pair.attrib['value']): int(pair.attrib['chance'])})
        for edge in root.find('map_info').find('in_edges').findall('edge'):
            self.in_edges.append(edge.attrib['value'])
        for edge in root.find('map_info').find('out_edges').findall('edge'):
            self.out_edges.append(edge.attrib['value'])

    def generate_drivers(self, out_file_vanet='vanet_routes.rou.xml', out_file_no_vanet='novanet_routes.rou.xml'):
        vanet_root = ET.Element('routes')
        no_vanet_root = ET.Element('routes')
        reflexes = []
        for key, value in self.delays.items():
            e = ET.SubElement(vanet_root, 'vType')
            e.set('id', str(key))
            e.set('tau', str(key+.1))
            e.set('actionStepLength', str(key))
            e.set('departSpeed', 'max')
            e = ET.SubElement(no_vanet_root, 'vType')
            e.set('id', str(key))
            e.set('tau', str(key+.1))
            e.set('actionStepLength', str(key))
            e.set('departSpeed', 'max')
            reflexes += [key]*value
        routes = []
        e = ET.SubElement(vanet_root, 'vType')
        e.set('id', '0')
        e.set('tau', '.1')
        e.set('actionStepLength', '0')
        e.set('departSpeed', 'max')
        for i in self.in_edges:
            for o in self.out_edges:
                e = ET.SubElement(vanet_root, 'route')
                e.set('id', i+o)
                e.set('edges', i+' '+o)
                e = ET.SubElement(no_vanet_root, 'route')
                e.set('id', i+o)
                e.set('edges', i+' '+o)
                routes.append(i+o)
        max_delay = 0
        for i in range(self.vQuantity):
            v = ET.SubElement(vanet_root, 'vehicle')
            nv = ET.SubElement(no_vanet_root, 'vehicle')
            v.set('id', str(i))
            v.set('depart', str(i))
            v.set('route', choice(routes))
            nv.set('id', str(i))
            nv.set('depart', str(i))
            nv.set('route', choice(routes))
            delay = choice(reflexes)
            if delay > max_delay:
                max_delay = delay
                v.set('type', str(delay))
            else:
                v.set('type', '0')
            nv.set('type', str(delay))
        ET.ElementTree(vanet_root).write(open(out_file_vanet, 'wb'))
        ET.ElementTree(no_vanet_root).write(open(out_file_no_vanet, 'wb'))


if __name__ == '__main__':
    a = DriverGenerator()
    print(a.vQuantity)
    print(a.delays)
    print(a.in_edges)
    a.generate_drivers()
