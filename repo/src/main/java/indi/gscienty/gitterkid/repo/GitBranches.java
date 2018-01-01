package indi.gscienty.gitterkid.repo;

import java.util.Iterator;
import java.util.List;
import java.util.Vector;
import java.util.function.Function;
import java.util.function.Predicate;

import com.sun.jna.Pointer;

import indi.gscienty.gitterkid.repo.nativelib.IRepositoryLibrary;

public class GitBranches
	implements Iterable<GitBranch>, Iterator<GitBranch>, IQueryable<GitBranch> {
	private Repository repository;
	private Pointer handle;
	private boolean iterateFirst;
	private IRepositoryLibrary lib;
	
	public GitBranches(Repository repository, Pointer branchesPointer) {
		this.handle = branchesPointer;
		this.lib = IRepositoryLibrary.Instance;
		this.iterateFirst = true;
		this.repository = repository;
		
		this.lib.git_branches_reset(this.handle);
	}
	
	@Override
	public boolean any(Predicate<GitBranch> predicate) {
		for (GitBranch branch : this) {
			if (predicate.test(branch)) {
				return true;
			}
		}
		return false;
	}

	@Override
	public boolean all(Predicate<GitBranch> predicate) {
		for (GitBranch branch : this) {
			if (predicate.test(branch) == false) {
				return false;
			}
		}
		return true;
	}

	@Override
	public GitBranch first(Predicate<GitBranch> predicate) {
		for (GitBranch branch : this) {
			if (predicate.test(branch)) {
				return branch;
			}
		}
		return null;
	}

	@Override
	public <R> List<R> filter(Function<GitBranch, R> transfer) {
		List<R> result = new Vector<>();
		for (GitBranch branch : this) {
			result.add(transfer.apply(branch));
		}
		return result;
	}

	@Override
	public boolean hasNext() {
		if (this.iterateFirst) {
			this.iterateFirst = false;
			return this.lib.git_branches_get_current(this.handle) != Pointer.NULL;
		}
		
		return this.lib.git_branches_move_next(this.handle) == 0;
	}

	@Override
	public GitBranch next() {
		return new GitBranch(
			this.repository,
			this.lib.git_branches_get_current(this.handle)
		);
	}

	@Override
	public Iterator<GitBranch> iterator() {
		this.lib.git_branches_reset(this.handle);
		return this;
	}

    /**
     * 析构方法
     */
    @Override
    protected void finalize() throws Throwable {
        this.lib.git_branches_dispose(this.handle);
        super.finalize();
    }
}