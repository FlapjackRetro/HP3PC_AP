# Run apworld/hp3pc/test/test_items.py without installing Archipelago's dependencies: a minimal BaseClasses (AP's own
# ItemClassification, a plain Item). Usage: python tools/test_apworld_items.py <Archipelago checkout> apworld/hp3pc
# AP's dependencies can't be installed here (no PyPI).
import ast, enum, sys, types, unittest, importlib.util
ap, world = sys.argv[1], sys.argv[2]
src = open(f"{ap}/BaseClasses.py").read()
node = next(n for n in ast.parse(src).body if isinstance(n, ast.ClassDef) and n.name == "ItemClassification")
bc = types.ModuleType("BaseClasses"); ns = {"IntFlag": enum.IntFlag}
exec(compile(ast.Module([node], []), "BaseClasses", "exec"), ns)
bc.ItemClassification = ns["ItemClassification"]
class Item:
    game = None
    def __init__(self, name, classification, code, player): self.name, self.classification, self.code, self.player = name, classification, code, player
bc.Item = Item; sys.modules["BaseClasses"] = bc
pkg = types.ModuleType("hp3pc"); pkg.__path__ = [world]; sys.modules["hp3pc"] = pkg
for name in ("items", "test", "test.test_items"):
    full = f"hp3pc.{name}"; path = f"{world}/{name.replace('.', '/')}" + ("/__init__.py" if name == "test" else ".py")
    spec = importlib.util.spec_from_file_location(full, path, submodule_search_locations=[f"{world}/test"] if name == "test" else None)
    m = importlib.util.module_from_spec(spec); sys.modules[full] = m; spec.loader.exec_module(m)
r = unittest.TextTestRunner(verbosity=2).run(unittest.defaultTestLoader.loadTestsFromModule(sys.modules["hp3pc.test.test_items"]))
it = sys.modules["hp3pc.items"]
print(len(it.ITEM_TABLE), "items, ids", min(it.ITEM_NAME_TO_ID.values()), "-", max(it.ITEM_NAME_TO_ID.values()))
print({g: len(v) for g, v in it.ITEM_NAME_GROUPS.items()})
sys.exit(not r.wasSuccessful())
