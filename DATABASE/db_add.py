import sqlite3

conn = sqlite3.connect('cubesat.db')\

c = conn.cursor()

# c.execute("""CREATE TABLE signals (
#         name text,
#         length integer,
#         units text,
#         byte_order integer,
#         factor real,
#         offset real,
#         minimum real,
#         maximum real
#        )""")

# c.execute("INSERT INTO signals VALUES ('vCell_1',16,'V',0,1000,0,0,5)")
# c.execute("INSERT INTO signals VALUES ('vCell_2',16,'V',0,1000,0,0,5)")
# c.execute("INSERT INTO signals VALUES ('vCell_3',16,'V',0,1000,0,0,5)")
# c.execute("INSERT INTO signals VALUES ('vCell_4',16,'V',0,1000,0,0,5)")

c.execute("SELECT factor FROM signals WHERE name='vCell_1'")
results = c.fetchone()
print(results[0]*8.234)

c.execute("SELECT * FROM signals")
print(c.fetchall())

conn.commit()

conn.close()