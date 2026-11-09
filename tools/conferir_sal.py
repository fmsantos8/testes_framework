"""Read this campaign's Logic 2 digital v4 captures without modifying originals.

This is an offline review utility, separate from the bench application's checks.
The supported internal layout is restricted to the captures from Logic 2.4.48.
Run from the repository root: python3 tools/conferir_sal.py [com_framework]
Writes review data under /tmp/sal-decoded; it never changes the source captures.
RLE reference: https://discuss.saleae.com/t/logic-2-capture-format-sal/1858/1
"""
from pathlib import Path
from array import array
from bisect import bisect_right, bisect_left
from dataclasses import dataclass
from collections import Counter
import struct, zipfile, json, re, csv, sys, hashlib, difflib, math

@dataclass
class Channel:
    rate: float
    initial: int
    edges: array
    samples: int

    def at(self, sample):
        return self.initial ^ (bisect_right(self.edges, sample) & 1)

def channel(data):
    assert data[:8] == b'<SALEAE>'
    assert struct.unpack_from('<II', data, 8) == (4, 100)
    assert data[16] == 1
    # v4: rate f64 at 17, clock timestamp at 25, chunk count u64 at 43.
    rate = struct.unpack_from('<d', data, 17)[0]
    chunks = struct.unpack_from('<Q', data, 43)[0]
    offset = 51
    edges = array('Q')
    last_end = 0
    current = None
    initial = None
    for i in range(chunks):
        start, end, state, size = struct.unpack_from('<QQHQ', data, offset)
        offset += 26
        assert start == last_end and end > start and state in (0, 1)
        if initial is None:
            initial = state
        elif current != state:
            edges.append(start)
        current = state
        pos, stop = offset, offset + size
        sample = start
        while pos < stop:
            value = data[pos]
            pos += 1
            assert value < 128
            if value >= 64:
                value &= 63
                while True:
                    byte = data[pos]
                    pos += 1
                    value = (value << 7) | (byte & 127)
                    if byte < 128:
                        break
            sample += value + 1
            assert sample <= end
            if sample < end:
                edges.append(sample)
                current ^= 1
        assert pos == stop and sample == end
        offset, last_end = stop, end
    assert offset == len(data)
    return Channel(rate, initial, edges, last_end)

def uart(ch, baud=115200):
    bit = ch.rate / baud
    frames = []
    i = 0
    while i < len(ch.edges):
        start = ch.edges[i]
        state = ch.initial ^ ((i + 1) & 1)
        if state or start + 10 * bit > ch.samples:
            i += 1
            continue
        if ch.at(start + .5 * bit):
            i += 1
            continue
        value = sum(ch.at(start + (1.5 + k) * bit) << k for k in range(8))
        frames.append({'start': start/ch.rate, 'end': (start+10*bit)/ch.rate,
                       'value': value, 'stop_ok': ch.at(start+9.5*bit) == 1})
        i = bisect_left(ch.edges, start + 9.75 * bit, i+1)
    return frames

def uart_lines(frames):
    data = bytes(f['value'] for f in frames)
    pattern = rb'(?:SAMPLE;temperature_mC=-?\d+;humidity_mpercent=\d+|ERROR;source=[A-Z0-9]+;code=\d+)\r\n'
    lines = []
    for m in re.finditer(pattern,data):
        lines.append({'start':frames[m.start()]['start'], 'end':frames[m.end()-1]['end'],
                      'text':m[0][:-2].decode(), 'stop_ok':all(f['stop_ok'] for f in frames[m.start():m.end()])})
    return data,lines

def i2c(scl,sda):
    assert scl.rate==sda.rate and scl.samples==sda.samples
    x=y=0; clk=scl.initial; dat=sda.initial
    transactions=[]; active=None; pending=None
    def finish(t, how):
        nonlocal active, pending
        if active is None: return
        bits=active.pop('bits')
        active['end']=t/scl.rate
        active['termination']=how
        active['bytes']=[sum(bits[k+j]<<(7-j) for j in range(8)) for k in range(0,len(bits)-8,9)]
        active['acks']=[bits[k+8]==0 for k in range(0,len(bits)-8,9)]
        active['remaining_bits']=len(bits)%9
        if active['bytes']:
            active['address']=active['bytes'][0]>>1
            active['read']=bool(active['bytes'][0]&1)
        transactions.append(active);active=None;pending=None
    while x<len(scl.edges) or y<len(sda.edges):
        cx=scl.edges[x] if x<len(scl.edges) else scl.samples+1
        dy=sda.edges[y] if y<len(sda.edges) else scl.samples+1
        t=min(cx,dy)
        nc=clk^(cx==t);nd=dat^(dy==t)
        if dy==t and clk and nc:
            if nd==0:
                finish(t,'RESTART')
                active={'start':t/scl.rate,'bits':[]};pending=None
            else:
                finish(t,'STOP')
        if cx==t and active is not None:
            if nc:
                pending=nd
            # Commit only completed clock pulses. The high level used to
            # prepare STOP/RESTART is not an extra data bit.
            elif pending is not None:
                active['bits'].append(pending);pending=None
        if cx==t: x+=1
        if dy==t: y+=1
        clk,dat=nc,nd
    finish(scl.samples,'CAPTURE_END')
    return transactions

def align_log(lines, rawlines):
    decoded = [line['text'] for line in lines]
    logged = [line.decode().removesuffix('\r\n') for line in rawlines]
    blocks = difflib.SequenceMatcher(None, decoded, logged, autojunk=False).get_opcodes()
    return {
        'matched_records': sum(j-i for tag,i,j,k,l in blocks if tag == 'equal'),
        'differences': [
            {'operation': tag, 'capture_records': lines[i:j], 'log_records': logged[k:l],
             'at_capture_boundary': i == 0 or j == len(lines)}
            for tag,i,j,k,l in blocks if tag != 'equal'
        ],
    }


def crc_word(data):
    crc = 0xff
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = ((crc << 1) ^ (0x31 if crc & 0x80 else 0)) & 0xff
    return crc


def validate_sensor(transactions, lines):
    reads = [t for t in transactions if t.get('address') == 0x44 and t['read']
             and len(t['bytes']) == 7 and t['acks'][0]]
    failures = []
    for read in reads:
        data = read['bytes'][1:]
        temperature = -45000 + (175000 * ((data[0]<<8)|data[1]) + 32767)//65535
        humidity = (100000 * ((data[3]<<8)|data[4]) + 32767)//65535
        expected = f'SAMPLE;temperature_mC={temperature};humidity_mpercent={humidity}'
        following = next((line for line in lines if line['start'] >= read['end']
                          and line['text'].startswith('SAMPLE')), None)
        previous = next((t for t in reversed(transactions)
                         if t['end'] <= read['start'] and t.get('address') == 0x44), None)
        checks = {
            'crc_temperature': crc_word(data[:2]) == data[2],
            'crc_humidity': crc_word(data[3:5]) == data[5],
            'acks_and_last_master_nack': read['acks'] == [True]*6 + [False],
            'stop': read['termination'] == 'STOP' and read['remaining_bits'] == 0,
            'separate_command': previous is not None and previous['bytes'] == [0x88,0x24,0]
                 and all(previous['acks']) and previous['termination'] == 'STOP'
                 and previous['remaining_bits'] == 0,
            'uart_value': following is not None and following['text'] == expected
                 and following['start']-read['end'] < 1 and following['stop_ok'],
        }
        if not all(checks.values()):
            failures.append({'start':read['start'], 'checks':checks})
    return {'complete_reads':len(reads), 'all_checks_pass':not failures, 'failures':failures}


def review_windows(transactions, lines, led):
    """Select reproducible examples; bounds are locators, not latency measures."""
    windows = []

    def add(key, protocol, first, last, detail, evidence):
        # Millisecond margins also include the final UART stop bit. The frame
        # decoder samples at 115200; it does not measure the actual baud rate.
        windows.append({'key':key, 'protocol':protocol,
                        'start_s':math.floor(first*1000)/1000,
                        'end_s':(math.ceil(last*1000)+1)/1000,
                        'observation':detail, 'evidence':evidence})

    def acquisition(line):
        read = next(t for t in reversed(transactions)
                    if t['end'] <= line['start'] and t.get('address') == 0x44
                    and t['read'] and len(t['bytes']) == 7 and t['acks'][0])
        write = next(t for t in reversed(transactions)
                     if t['end'] <= read['start'] and t.get('address') == 0x44)
        return write, read

    # The first short LED pulse marks initialization/reset in these captures.
    threshold = led.edges[1]/led.rate + 2 if len(led.edges)>1 else 5
    normal = next(line for line in lines if line['start']>threshold
                  and line['text'].startswith('SAMPLE'))
    write, read = acquisition(normal)
    oled = next(t for t in transactions if t['start']>read['end']
                and t.get('address') == 0x3c and len(t['bytes'])>100 and all(t['acks']))
    add('normal_sensor','I2C',write['start'],read['end'],
        '0x44: escrita 24 00, STOP, leitura separada de seis bytes; dois CRC válidos; NACK final do mestre.',[write,read])
    add('normal_oled','I2C',oled['start'],oled['end'],
        '0x3C: escrita de dados do OLED com ACK e STOP.',[oled])
    add('normal_uart','UART',normal['start'],normal['end'],
        'SAMPLE completo com CR LF; valores correspondem aos bytes da leitura I2C.',[normal])

    display_errors = [line for line in lines if 'source=SSD1306' in line['text']]
    sensor_errors = [line for line in lines if 'source=SHT31D' in line['text']
                     and line['start']<display_errors[0]['start']]
    groups = []
    for line in sensor_errors:
        if not groups or line['start']-groups[-1][-1]['start']>3:
            groups.append([])
        groups[-1].append(line)
    main = max(groups,key=len)
    assert not any(main[0]['start'] <= line['start'] <= main[-1]['end']
                   and line['text'].startswith('SAMPLE') for line in lines)
    failed = next(t for t in reversed(transactions) if t['end']<=main[0]['start']
                  and t.get('address') == 0x44 and t['acks'] and not t['acks'][0])
    add('sensor_fault','I2C/UART',failed['start'],main[-1]['end'],
        f'Endereço 0x44 sem ACK; {len(main)} ERROR do sensor; nenhuma SAMPLE nesse bloco.',
        [failed,main[0],main[-1]])
    resumed = [line for line in lines if line['start']>main[-1]['end']
               and line['text'].startswith('SAMPLE')][:2]
    recovery_write,recovery_read = acquisition(resumed[0])
    add('sensor_recovery','I2C/UART',recovery_write['start'],resumed[-1]['end'],
        'Comando e leitura com CRC válido retomados; duas SAMPLE completas após o bloco principal.',
        [recovery_write,recovery_read,*resumed])

    # STM32F407 conventional: keep the extra shared-bus intervention separate.
    later_sensor = next((line for line in lines if line['start']>display_errors[0]['start']
                         and 'source=SHT31D' in line['text']),None)
    clean_errors = [line for line in display_errors
                    if later_sensor is None or line['start']<later_sensor['start']]
    failed_oled = next(t for t in reversed(transactions) if t['end']<=clean_errors[0]['start']
                       and t.get('address') == 0x3c and not all(t['acks']))
    sample_count = sum(clean_errors[0]['start']<=line['start']<=clean_errors[-1]['end']
                       and line['text'].startswith('SAMPLE') for line in lines)
    add('display_fault','I2C/UART',failed_oled['start'],clean_errors[-1]['end'],
        f'Falha de ACK do OLED; {len(clean_errors)} ERROR do display; {sample_count} SAMPLE intercaladas.',
        [failed_oled,clean_errors[0],clean_errors[-1]])
    oled_ok = next(t for t in transactions if t['start']>display_errors[-1]['end']
                   and t.get('address') == 0x3c and len(t['bytes'])>100 and all(t['acks']))
    after = [line for line in lines if line['start']>display_errors[-1]['end']
             and line['text'].startswith('SAMPLE')][:2]
    add('display_recovery','I2C/UART',min(oled_ok['start'],after[0]['start']),
        max(oled_ok['end'],after[-1]['end']),
        'Escrita de dados do OLED com ACK retomada; duas SAMPLE completas após o último ERROR do display.',
        [oled_ok,*after])
    return windows


def main(p):
    with zipfile.ZipFile(p) as z:
        meta=json.loads(z.read('meta.json'))
        channels=[channel(z.read(f'digital-{i}.bin')) for i in (0,1,2,3,4)]
    scl,sda,tx,led,button=channels
    csvp=p.parent/'digital.csv'
    csv_check=None
    if csvp.exists():
        rows=list(csv.DictReader(csvp.open()))
        observed=[(0.,led.initial)]+[(s/led.rate,led.initial^((i+1)&1)) for i,s in enumerate(led.edges)]
        observed.append((led.samples/led.rate,led.at(led.samples-1)))
        expected=[(float(r['Time [s]']),int(r['LED'])) for r in rows]
        csv_check=observed==expected
        assert csv_check
    frames=uart(tx); data,lines=uart_lines(frames)
    session=json.loads((p.parent/'sessao.json').read_text())
    raw=(p.parent/session['uart_logs'][-1]['raw']).read_bytes()
    rawlines=re.findall(rb'(?:SAMPLE;temperature_mC=-?\d+;humidity_mpercent=\d+|ERROR;source=[A-Z0-9]+;code=\d+)\r\n',raw)
    decodedlines=[(l['text']+'\r\n').encode() for l in lines]
    joined=b''.join(decodedlines)
    txs=i2c(scl,sda)
    summary={'capture':str(p),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),
             'duration_s':scl.samples/scl.rate,'rate_hz':scl.rate,'led_csv_exact':csv_check,
             'uart_frames':len(frames),'uart_frame_errors':sum(not f['stop_ok'] for f in frames),
             'uart_lines':len(lines),'uart_counts':dict(Counter(l['text'].split(';')[0] if l['text'].startswith('SAMPLE') else l['text'] for l in lines)),
             'raw_contiguous':joined in b''.join(rawlines),'raw_lines':len(rawlines),
             'uart_first':lines[:2],'uart_last':lines[-2:],
             'i2c_transactions':len(txs),'i2c_addresses':dict(Counter(hex(t['address']) for t in txs if 'address'in t)),
             'i2c_partial':sum(t['remaining_bits']!=0 for t in txs),
             'i2c_terminations':dict(Counter(t['termination'] for t in txs)),
             'first_transactions':txs[:2]}
    summary['uart_log_alignment'] = align_log(lines, rawlines)
    summary['sensor_validation'] = validate_sensor(txs, lines)
    assert summary['sensor_validation']['all_checks_pass']
    summary['uart_frame_end_method'] = 'Start + 10 bits a 115200; janelas incluem margem, sem medir baud real.'
    windows = review_windows(txs, lines, led)
    out=Path('/tmp/sal-decoded');out.mkdir(exist_ok=True)
    tag=p.parts[2]+'_'+p.parts[3]
    (out/(tag+'.json')).write_text(json.dumps({'summary':summary,'windows':windows,'uart':lines,'i2c':txs},ensure_ascii=False,indent=2))
    (out/(tag+'_uart.bin')).write_bytes(data)
    print(json.dumps({k:v for k,v in summary.items() if k not in ('first_transactions','uart_first','uart_last')},ensure_ascii=False))

if __name__=='__main__':
    for p in sorted(Path('evidencias/bancada').rglob('*.sal')):
        if len(sys.argv)==1 or any(a in str(p) for a in sys.argv[1:]): main(p)
