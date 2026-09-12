/*
 * A simple kernel module that reports context-switch counts for a given process PID. 
 * It reads task_struct to printout the number of voluntary and involuntary context switches for that process.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/rcupdate.h>

static int target_pid = 0;
module_param(target_pid, int, 0644);
MODULE_PARM_DESC(target_pid, "PID to report context-switch counts for");

/* ---- /proc/csprobe reader ---- */
static int cs_show(struct seq_file *m, void *v)
{
    struct pid *p;
    struct task_struct *task;
    int tp = READ_ONCE(target_pid);

    if (tp == 0) {
        seq_printf(m, "target_pid is 0 -- set it first:\n"
                      "  echo <pid> | sudo tee "
                      "/sys/module/csprobe/parameters/target_pid\n");
        return 0;
    }

    rcu_read_lock();
    p = find_get_pid(tp);
    task = p ? pid_task(p, PIDTYPE_PID) : NULL;

    if (task) {
        seq_printf(m, "comm            : %s\n", task->comm);
        seq_printf(m, "pid             : %d\n", tp);
        seq_printf(m, "voluntary   (nvcsw)  : %lu\n", task->nvcsw);
        seq_printf(m, "involuntary (nivcsw) : %lu\n", task->nivcsw);
    } else {
        seq_printf(m, "pid %d not found (task exited?)\n", tp);
    }
    rcu_read_unlock();
    if (p)
        put_pid(p);

    return 0;
}

static int cs_open(struct inode *inode, struct file *file)
{
    return single_open(file, cs_show, NULL);
}

static const struct proc_ops cs_ops = {
    .proc_open    = cs_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static struct proc_dir_entry *cs_entry;

static int __init csprobe_init(void)
{
    cs_entry = proc_create("csprobe", 0444, NULL, &cs_ops);
    if (!cs_entry) {
        pr_err("csprobe: failed to create /proc/csprobe\n");
        return -ENOMEM;
    }
    pr_info("csprobe: loaded\n");
    return 0;
}

static void __exit csprobe_exit(void)
{
    if (cs_entry)
        remove_proc_entry("csprobe", NULL);
    pr_info("csprobe: unloaded\n");
}

module_init(csprobe_init);
module_exit(csprobe_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ID1200/ID1206");
MODULE_DESCRIPTION("Report per-task context-switch counts (nvcsw/nivcsw)");
